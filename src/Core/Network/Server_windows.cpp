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

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>

#include "Core/Packet.h"
#include "Core/ThreadSafeQueue.h"
#include "Util/PacketUtil.h"

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
  clientid_t clientID;

  ClientInfo(SOCKET s, clientid_t id)
      : socket(s),
        clientID(id),
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
  std::atomic<clientid_t> nextClientID{1};
  std::unordered_map<SOCKET, ClientInfo *> socketToInfoMap;
  std::unordered_map<clientid_t, ClientInfo *> idToInfoMap;
  ThreadSafeQueue<RecvPacket> *recvQueue;
  ThreadSafeQueue<SendRequest> *sendQueue;
  bool bIsRunning;

  static void PacketSendHelper(ClientInfo *client, char *sendbuffer,
                               std::size_t packet_size) {
    ZeroMemory(&(client->pSendOverlapped.get()->overlapped),
               sizeof(WSAOVERLAPPED));
    std::memcpy(client->pSendOverlapped->messageBuffer, sendbuffer,
                packet_size);
    client->pSendOverlapped->dataBuf.len = packet_size;
    client->pSendOverlapped->operationType = IO_OPERATION::SEND;

    client->AddRef();

    int res = WSASend(client->socket, &(client->pSendOverlapped->dataBuf), 1,
                      &(client->pSendOverlapped->bytesSent), 0,
                      (LPOVERLAPPED)client->pSendOverlapped.get(), nullptr);

    std::cout << "sent bytes : ";
    for (int i = 0; i < client->pSendOverlapped->dataBuf.len; ++i) {
      std::cout << std::hex << client->pSendOverlapped->messageBuffer[i];
    }
    std::cout << std::endl;
    if (res == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
      std::cerr << "WSASend failed:" << WSAGetLastError() << std::endl;
      client->Release();
    }
  }

  void WorkerThread() {
    DWORD recvByteCnt{0};
    DWORD dwFlags{0};
    LPOVERLAPPED pOverlapped = nullptr;
    ClientInfo *completionKey = nullptr;
    while (true) {
      BOOL res = GetQueuedCompletionStatus(iocpHandle, &recvByteCnt,
                                           (ULONG_PTR *)&completionKey,
                                           &pOverlapped, INFINITE);

      // Shutting down thread
      if ((ULONG_PTR)completionKey == SHUT_DOWN_KEY) {
        return;
      }

      else if ((ULONG_PTR)completionKey == WAKE_UP_KEY) {
        SendRequest request;
        std::vector<char> sendBuffer;

        // send every request inside queue
        while (sendQueue->TryPop(request)) {
          uint8_t *p = request.packet.get();

          std::size_t packetSize;
          PACKET packetId;
          util::GetHeader(p, packetId, packetSize);

          sendBuffer.resize(packetSize);
          std::memcpy(sendBuffer.data(), request.packet.get(), packetSize);

          // Process Unicast
          if (request.type == ESendType::UNICAST) {
            // minimize critical section
            AcquireSRWLockShared(&clientMapSRW);
            auto it = idToInfoMap.find(request.targetClientId);
            if (it != idToInfoMap.end()) {
              ClientInfo *client = it->second;
              ReleaseSRWLockShared(&clientMapSRW);
              PacketSendHelper(client, sendBuffer.data(), packetSize);
            } else {
              // instantly release lock if not found
              ReleaseSRWLockShared(&clientMapSRW);
            }
          }

          // Process Broadcast
          else if (request.type == ESendType::BROADCAST) {
            std::vector<ClientInfo *> clientsToSend;
            clientsToSend.reserve(socketToInfoMap.size());

            // minimize critical section
            AcquireSRWLockShared(&clientMapSRW);
            for (auto it = socketToInfoMap.begin(); it != socketToInfoMap.end();
                 ++it) {
              if (it->second == nullptr) continue;
              clientsToSend.push_back(it->second);
            }
            ReleaseSRWLockShared(&clientMapSRW);

            for (auto client : clientsToSend) {
              PacketSendHelper(client, sendBuffer.data(), packetSize);
            }
          }
        }
        // Sending Job Done - Back to waiting threadpool
        continue;
      }

      // Normal Socket Packet Recv
      SOCKET_OVERLAPPED *pSocketOverlapped =
          CONTAINING_RECORD(pOverlapped, SOCKET_OVERLAPPED, overlapped);

      // Socket failed - disconnect client
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
        socketToInfoMap.erase(completionKey->socket);
        idToInfoMap.erase(completionKey->clientID);
        ReleaseSRWLockExclusive(&clientMapSRW);

        completionKey->Release();  // disconnected so release
        continue;
      }

      // Received Actual Packet
      if (pSocketOverlapped->operationType == IO_OPERATION::RECEIVE) {
        std::cout << "Bytes received: " << recvByteCnt << std::endl;
        std::cout << "received bytes : ";
        for (int i = 0; i < recvByteCnt; ++i) {
          std::cout << std::hex << pSocketOverlapped->messageBuffer[i];
        }
        std::cout << std::endl;

        RecvPacket recvPacket;
        recvPacket.senderClientId = completionKey->clientID;
        recvPacket.packet = std::make_unique<uint8_t[]>(recvByteCnt);
        std::memcpy(recvPacket.packet.get(), pSocketOverlapped->messageBuffer,
                    recvByteCnt);

        recvQueue->Push(std::move(recvPacket));

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
          std::cerr << "WSARecv failed : " << WSAGetLastError() << std::endl;
        }
      }
      // Received nothing, just awaken by WSASend of myself
      else {
        if (completionKey->pSendOverlapped.get() != pSocketOverlapped) {
          std::cerr << "Client and overlap object mismatch!" << std::endl;
        }

        std::cout << "Bytes sent: " << pSocketOverlapped->bytesSent
                  << std::endl;

        completionKey->Release();
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

  bool Init(ThreadSafeQueue<RecvPacket> *recvQ,
            ThreadSafeQueue<SendRequest> *sendQ) override {
    WSADATA wsaData;
    InitializeSRWLock(&clientMapSRW);
    int res = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (res != 0) {
      std::cerr << "Can't Initialize winsock." << std::endl;
      return false;
    }

    recvQueue = recvQ;
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

      ClientInfo *pClientInfo = new ClientInfo(clientSocket, nextClientID++);

      AcquireSRWLockExclusive(&clientMapSRW);
      socketToInfoMap[clientSocket] = pClientInfo;
      idToInfoMap[pClientInfo->clientID] = pClientInfo;
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
        std::cerr << "WSARecv failed : " << WSAGetLastError() << std::endl;
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

    for (std::size_t i = 0; i < threadPool.size(); ++i)
      CloseHandle(threadPool[i]);

    CloseHandle(iocpHandle);

    AcquireSRWLockExclusive(&clientMapSRW);
    for (auto const &[sock, clientInfo] : socketToInfoMap) {
      closesocket(sock);
    }
    ReleaseSRWLockExclusive(&clientMapSRW);

    socketToInfoMap.clear();
    idToInfoMap.clear();
    WSACleanup();
    std::cout << "IOCP server stopped." << std::endl;
  }
};

#endif  // _WIN32