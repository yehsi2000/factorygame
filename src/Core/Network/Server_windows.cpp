#include "Core/ServerImpl.h"

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <WS2tcpip.h>
#include <WinSock2.h>
#include <memory.h>
#include <process.h>
#include <windows.h>

#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>

#include "Core/Packet.h"
#include "Core/ThreadSafeQueue.h"

namespace {
constexpr DWORD MAX_BUFFER = 1024;
constexpr ULONG_PTR SHUT_DOWN_KEY = 0ul;
constexpr ULONG_PTR WAKE_UP_KEY = 1ul;
constexpr USHORT SERVER_PORT = 27015;
enum class IO_OPERATION { RECEIVE, SEND };

struct SOCKET_OVERLAPPED {
  WSAOVERLAPPED overlapped;
  IO_OPERATION operationType;
  CHAR messageBuffer[MAX_BUFFER];
  WSABUF dataBuf;
  DWORD bytesRecv = 0;
  DWORD bytesSent = 0;
  SOCKET_OVERLAPPED(IO_OPERATION operationType)
      : overlapped(),
        operationType(operationType),
        messageBuffer(),
        dataBuf{MAX_BUFFER, messageBuffer} {}
};

struct ClientInfo {
  SOCKET socket;
  std::unique_ptr<SOCKET_OVERLAPPED> pSendOverlapped;
  std::unique_ptr<SOCKET_OVERLAPPED> pRecvOverlapped;

  DWORD refCount;

  ClientInfo(SOCKET s)
      : socket(s),
        refCount(1),
        pSendOverlapped(nullptr),
        pRecvOverlapped(nullptr) {}

  void AddRef() { InterlockedIncrement(&refCount); }

  void Release() {
    if (InterlockedDecrement(&refCount) == 0) {
      closesocket(this->socket);
      pRecvOverlapped.reset();
      pSendOverlapped.reset();
      delete this;
    }
  }
};
}  // anonymous namespace

class WindowsServerImpl : public ServerImpl {
  HANDLE iocpHandle = INVALID_HANDLE_VALUE;
  HANDLE serverThreadHandle = INVALID_HANDLE_VALUE;
  SOCKET listenSocket;
  SRWLOCK clientMapSRW;
  std::vector<HANDLE> threadPool;
  std::unordered_map<SOCKET, ClientInfo *> clientPtrMap;
  ThreadSafeQueue<PacketPtr> *packetQueue;
  ThreadSafeQueue<SendRequest> *sendQueue;
  std::vector<char> sendBuffer;
  bool bIsRunning;

  void WorkerThread() {
    DWORD recvByteCnt{0};
    DWORD dwFlags{0};
    LPOVERLAPPED pOverlapped = nullptr;
    ClientInfo *completionKey = nullptr;
    while (true) {
      BOOL res = GetQueuedCompletionStatus(iocpHandle, &recvByteCnt,
                                           (ULONG_PTR *)&completionKey,
                                           &pOverlapped, INFINITE);

      if ((ULONG_PTR)completionKey == SHUT_DOWN_KEY) {
        break;
      }

      else if ((ULONG_PTR)completionKey == WAKE_UP_KEY) {
        SendRequest request;
        // 큐에 쌓인 요청을 모두 처리
        while (sendQueue->TryPop(request)) {
          PacketHeader *packetHeader =
              reinterpret_cast<PacketHeader *>(request.packet.get());

          sendBuffer.clear();
          sendBuffer.resize(packetHeader->packet_size);
          memcpy(sendBuffer.data(), request.packet.get(),
                 packetHeader->packet_size);

          if (request.type == ESendType::UNICAST) {
            AcquireSRWLockShared(&clientMapSRW);

            auto it = clientPtrMap.find(request.targetClientId);

            if (it != clientPtrMap.end()) {
              ClientInfo *client = it->second;

              ZeroMemory(&client->pSendOverlapped.get()->overlapped, sizeof(WSAOVERLAPPED));
              memcpy(client->pSendOverlapped->messageBuffer, sendBuffer.data(),
                     packetHeader->packet_size);
              client->pSendOverlapped->dataBuf.len = packetHeader->packet_size;
              client->pSendOverlapped->operationType = IO_OPERATION::SEND;

              client->AddRef();

              res =
                  WSASend(client->socket, &client->pSendOverlapped->dataBuf, 1,
                          &client->pSendOverlapped->bytesSent, 0,
                          (LPOVERLAPPED)client->pSendOverlapped.get(), nullptr);

              if (res == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
                std::cerr << "WSASend failed:" << WSAGetLastError()
                          << std::endl;
                client->Release();
              }
            }

            ReleaseSRWLockShared(&clientMapSRW);
          } else if (request.type == ESendType::BROADCAST) {
            AcquireSRWLockShared(&clientMapSRW);

            for (auto it = clientPtrMap.begin(); it != clientPtrMap.end();
                 ++it) {
              if (it->second == nullptr) continue;

              ClientInfo *client = it->second;


              ZeroMemory(&client->pSendOverlapped.get()->overlapped, sizeof(WSAOVERLAPPED));
              memcpy(client->pSendOverlapped->messageBuffer, sendBuffer.data(),
                     packetHeader->packet_size);
              client->pSendOverlapped->dataBuf.len = packetHeader->packet_size;
              client->pSendOverlapped->operationType = IO_OPERATION::SEND;

              client->AddRef();

              res =
                  WSASend(client->socket, &client->pSendOverlapped->dataBuf, 1,
                          &client->pSendOverlapped->bytesSent, 0,
                          (LPOVERLAPPED)client->pSendOverlapped.get(), nullptr);

              if (res == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
                std::cerr << "WSASend failed:" << WSAGetLastError()
                          << std::endl;
                client->Release();
              }
            }

            ReleaseSRWLockShared(&clientMapSRW);
          }
        }
        continue;
      }

      // Normal Socket Packet Recv
      SOCKET_OVERLAPPED *pSocketOverlapped =
          CONTAINING_RECORD(pOverlapped, SOCKET_OVERLAPPED, overlapped);

      if (res == 0 || recvByteCnt == 0) {
        if (res == 0) {
          std::cerr << "GetQueuedCompletionStatus failed for socket "
                    << completionKey->socket << ": " << GetLastError()
                    << std::endl;
        } else {
          std::cerr << "Client disconnected: " << completionKey->socket
                    << std::endl;
        }

        AcquireSRWLockExclusive(&clientMapSRW);
        clientPtrMap.erase(completionKey->socket);
        ReleaseSRWLockExclusive(&clientMapSRW);

        completionKey->Release();  // 그 후에 Release 호출
        continue;
      }

      if (recvByteCnt == 0) {
        std::cerr << "Client disconnected:" << GetLastError() << std::endl;
        completionKey->Release();
        continue;
      } else {
        if (pSocketOverlapped->operationType == IO_OPERATION::RECEIVE) {
          std::cout << "Bytes received: " << recvByteCnt << std::endl;

          auto packetBuffer = std::make_unique<uint8_t[]>(recvByteCnt);
          memcpy(packetBuffer.get(), pSocketOverlapped->messageBuffer,
                 recvByteCnt);
          packetQueue->Push(std::move(packetBuffer));

          ZeroMemory(&pSocketOverlapped->overlapped, sizeof(WSAOVERLAPPED));
          pSocketOverlapped->dataBuf.len = MAX_BUFFER;
          pSocketOverlapped->dataBuf.buf = pSocketOverlapped->messageBuffer;
          pSocketOverlapped->bytesRecv = 0;
          pSocketOverlapped->operationType = IO_OPERATION::RECEIVE;

          ZeroMemory(pSocketOverlapped->messageBuffer, MAX_BUFFER);

          completionKey->AddRef();

          res = WSARecv(completionKey->socket, &pSocketOverlapped->dataBuf, 1,
                        &recvByteCnt, &dwFlags, &pSocketOverlapped->overlapped,
                        nullptr);

          if (res == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
            std::cerr << "WSARecv failed:" << WSAGetLastError() << std::endl;
          }
        } else {
          // IO_OPERATION SEND
          if (completionKey->pSendOverlapped.get() == pSocketOverlapped) {
            std::cout << "Bytes sent: " << pSocketOverlapped->bytesSent
                      << std::endl;
          }
          completionKey->Release();
        }
      }
    }
  }

  static unsigned WINAPI ThreadEntry(void *p) {
    WindowsServerImpl *pServer = static_cast<WindowsServerImpl *>(p);
    pServer->WorkerThread();
    return 0;
  }

  static unsigned WINAPI StartServer(void *p) {
    WindowsServerImpl *pServer = static_cast<WindowsServerImpl *>(p);
    pServer->StartThread();
    return 0;
  }

  bool CreateThreadPool() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    DWORD nThreadCnt = sysInfo.dwNumberOfProcessors * 2;

    threadPool = std::vector<HANDLE>(nThreadCnt);

    for (std::size_t i = 0; i < nThreadCnt; i++) {
      threadPool[i] = (HANDLE)_beginthreadex(
          nullptr, 0, &WindowsServerImpl::ThreadEntry, this, 0, nullptr);
      if (threadPool[i] == nullptr) {
        return false;
      }
    }

    return true;
  }

 public:
  WindowsServerImpl() : iocpHandle(NULL) {}
  ~WindowsServerImpl() override { Stop(); }

  bool Init(ThreadSafeQueue<PacketPtr> *packQ,
            ThreadSafeQueue<SendRequest> *sendQ) override {
    WSADATA wsaData;
    InitializeSRWLock(&clientMapSRW);
    int res = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (res != 0) {
      std::cerr << "Can't Initialize winsock." << std::endl;
      return false;
    }

    packetQueue = packQ;
    sendQueue = sendQ;

    listenSocket =
        WSASocket(AF_INET, SOCK_STREAM, 0, nullptr, 0, WSA_FLAG_OVERLAPPED);
    if (listenSocket == INVALID_SOCKET) {
      std::cerr << "socket failed with error:" << WSAGetLastError()
                << std::endl;
      return false;
    }

    SOCKADDR_IN serverAddr{};
    serverAddr.sin_family = PF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

    res = bind(listenSocket, (sockaddr *)&serverAddr, sizeof(SOCKADDR_IN));
    if (res == SOCKET_ERROR) {
      std::cerr << "Fail to bind socket" << std::endl;
      closesocket(listenSocket);
      WSACleanup();
      return false;
    }

    res = listen(listenSocket, SOMAXCONN);
    if (res == SOCKET_ERROR) {
      std::cerr << "Fail to listen." << std::endl;
      closesocket(listenSocket);
      WSACleanup();
      return false;
    }

    iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
    if (iocpHandle == nullptr) {
      std::cerr << "CreateIoCompletionPort failed:" << GetLastError()
                << std::endl;
      closesocket(listenSocket);
      WSACleanup();
      return false;
    }

    if (!CreateThreadPool()) {
      std::cerr << "CreateThreadPool failed." << std::endl;
      CloseHandle(iocpHandle);
      closesocket(listenSocket);
      WSACleanup();
      return false;
    }
    return true;
  }

  void Start() override {
    bIsRunning = true;
    if (serverThreadHandle != INVALID_HANDLE_VALUE) return;
    serverThreadHandle = (HANDLE)_beginthreadex(
        nullptr, 0, &WindowsServerImpl::StartServer, this, 0, nullptr);
  }

  void StartThread() {
    std::cout << "IOCP server started." << std::endl;

    while (bIsRunning) {
      SOCKADDR_IN clientAddr;
      int addrLen = sizeof(SOCKADDR_IN);

      SOCKET clientSocket = WSAAccept(listenSocket, (sockaddr *)&clientAddr,
                                      &addrLen, nullptr, 0);

      if (clientSocket == INVALID_SOCKET) {
        std::cerr << "fail to accept." << std::endl;
        continue;
      }
      std::cout << "Client connected." << std::endl;

      ClientInfo *pClientInfo = new ClientInfo(clientSocket);

      AcquireSRWLockExclusive(&clientMapSRW);
      clientPtrMap[clientSocket] = pClientInfo;
      ReleaseSRWLockExclusive(&clientMapSRW);

      pClientInfo->pRecvOverlapped =
          std::make_unique<SOCKET_OVERLAPPED>(IO_OPERATION::RECEIVE);
      pClientInfo->pSendOverlapped =
          std::make_unique<SOCKET_OVERLAPPED>(IO_OPERATION::SEND);

      CreateIoCompletionPort((HANDLE)clientSocket, iocpHandle,
                             (ULONG_PTR)pClientInfo, 0);

      DWORD dwFlags = 0;

      pClientInfo->AddRef();
      int res = WSARecv(clientSocket, &pClientInfo->pRecvOverlapped->dataBuf, 1,
                        &pClientInfo->pRecvOverlapped->bytesRecv, &dwFlags,
                        &pClientInfo->pRecvOverlapped->overlapped, nullptr);

      if (res == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
        std::cerr << "WSARecv failed:" << WSAGetLastError() << std::endl;
        pClientInfo->Release();
        continue;
      }
    }
    
  }

  void StartSend() override {
    PostQueuedCompletionStatus(iocpHandle, 0, WAKE_UP_KEY, nullptr);
  }

  void Stop() override {
    bIsRunning = false;
    closesocket(listenSocket);
    CloseHandle(serverThreadHandle);

    for (std::size_t i = 0; i < threadPool.size(); ++i)
      PostQueuedCompletionStatus(iocpHandle, 0, SHUT_DOWN_KEY, NULL);

    WaitForMultipleObjects(static_cast<DWORD>(threadPool.size()),
                           threadPool.data(), TRUE, INFINITE);

    for (std::size_t i = 0; i < threadPool.size(); ++i) CloseHandle(threadPool[i]);

    CloseHandle(iocpHandle);

    AcquireSRWLockExclusive(&clientMapSRW);
    for (auto const &[sock, clientInfo] : clientPtrMap) {
      closesocket(sock);
    }
    ReleaseSRWLockExclusive(&clientMapSRW);

    clientPtrMap.clear();
    WSACleanup();
    std::cout << "IOCP server stopped." << std::endl;
  }
};

#endif  // _WIN32