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

#include <cstdlib>
#include <iostream>
#include <list>
#include <vector>


namespace {
constexpr DWORD MAX_BUFFER = 1024;
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
  SOCKET_OVERLAPPED *pSendOverlapped;
  SOCKET_OVERLAPPED *pRecvOverlapped;

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
      delete pRecvOverlapped;
      delete pSendOverlapped;
      delete this;
    }
  }
};
}  // anonymous namespace

class WindowsServerImpl : public ServerImpl {
  HANDLE iocpHandle = INVALID_HANDLE_VALUE;
  HANDLE serverThreadHandle = INVALID_HANDLE_VALUE;
  SOCKET listenSocket;
  std::vector<HANDLE> threadPool;
  bool bIsRunning;

  void WorkerThread() {
    DWORD recvByteCnt{0};
    DWORD dwFlags{0};
    LPOVERLAPPED pOverlapped = nullptr;
    ClientInfo *pClientInfo = nullptr;
    while (true) {
      BOOL res = GetQueuedCompletionStatus(iocpHandle, &recvByteCnt,
                                           (ULONG_PTR *)&pClientInfo,
                                           &pOverlapped, INFINITE);

      SOCKET_OVERLAPPED *pSocketOverlapped =
          CONTAINING_RECORD(pOverlapped, SOCKET_OVERLAPPED, overlapped);

      if (res == 0) {
        std::cerr << "GetQueuedCompletionStatus failed:" << GetLastError()
                  << std::endl;
        pClientInfo->Release();
        continue;
      }

      if (pClientInfo == 0 && pOverlapped == 0) {
        // Shutdown signal
        break;
      }

      if (recvByteCnt == 0) {
        std::cerr << "Client disconnected:" << GetLastError() << std::endl;
        pClientInfo->Release();
        continue;
      } else {
        if (pSocketOverlapped->operationType == IO_OPERATION::RECEIVE) {
          std::cout << "Bytes received: " << recvByteCnt << std::endl;

          memcpy(pClientInfo->pSendOverlapped->messageBuffer,
                 pClientInfo->pRecvOverlapped->messageBuffer, recvByteCnt);

          pClientInfo->AddRef();

          res = WSASend(pClientInfo->socket,
                        &pClientInfo->pSendOverlapped->dataBuf, 1,
                        &pClientInfo->pSendOverlapped->bytesSent, 0,
                        (LPOVERLAPPED)pClientInfo->pSendOverlapped, nullptr);

          if (res == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
            std::cerr << "WSASend failed:" << WSAGetLastError() << std::endl;
          }

          ZeroMemory(&pSocketOverlapped->overlapped, sizeof(WSAOVERLAPPED));
          pSocketOverlapped->dataBuf.len = MAX_BUFFER;
          pSocketOverlapped->dataBuf.buf = pSocketOverlapped->messageBuffer;
          pSocketOverlapped->bytesRecv = 0;
          pSocketOverlapped->operationType = IO_OPERATION::RECEIVE;

          ZeroMemory(pSocketOverlapped->messageBuffer, MAX_BUFFER);

          pClientInfo->AddRef();

          res = WSARecv(pClientInfo->socket, &pSocketOverlapped->dataBuf, 1,
                        &recvByteCnt, &dwFlags, &pSocketOverlapped->overlapped,
                        nullptr);

          if (res == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
            std::cerr << "WSARecv failed:" << WSAGetLastError() << std::endl;
          }
        } else {
          if (pClientInfo->pSendOverlapped == pSocketOverlapped) {
            std::cout << "Bytes sent: " << pSocketOverlapped->bytesSent
                      << std::endl;
          }
          pClientInfo->Release();
        }
      }
    }
  }

  static unsigned WINAPI ThreadEntry(void* p){
    WindowsServerImpl* pServer = static_cast<WindowsServerImpl*>(p);
    pServer->WorkerThread();
    return 0;
  }

  static unsigned WINAPI StartServer(void* p){
    WindowsServerImpl* pServer = static_cast<WindowsServerImpl*>(p);
    pServer->StartThread();
    return 0;
  }

  bool CreateThreadPool() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    DWORD nThreadCnt = sysInfo.dwNumberOfProcessors * 2;

    threadPool = std::vector<HANDLE>(nThreadCnt);

    for (size_t i = 0; i < nThreadCnt; i++) {
      threadPool[i] = (HANDLE)_beginthreadex(nullptr, 0, &WindowsServerImpl::ThreadEntry, this, 0, nullptr);
      if (threadPool[i] == nullptr) {
        return false;
      }
    }

    return true;
  }

 public:
  WindowsServerImpl() : iocpHandle(NULL) {}
  ~WindowsServerImpl() override { Stop(); }

  bool Init() override {
    WSADATA wsaData;
    int res = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (res != 0) {
      std::cerr << "Can't Initialize winsock." << std::endl;
      return false;
    }

    SOCKET listenSocket;

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

  void Start() override{
    if(serverThreadHandle == INVALID_HANDLE_VALUE) return;
    serverThreadHandle = (HANDLE)_beginthreadex(nullptr, 0, &WindowsServerImpl::StartServer, this, 0, nullptr);
  }

  void StartThread() {
    while (true) {
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

      pClientInfo->pRecvOverlapped =
          new SOCKET_OVERLAPPED(IO_OPERATION::RECEIVE);
      pClientInfo->pSendOverlapped = new SOCKET_OVERLAPPED(IO_OPERATION::SEND);

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
    std::cout << "IOCP server started." << std::endl;
  }

  void Stop() override {
    closesocket(listenSocket);
    CloseHandle(serverThreadHandle);

    for (size_t i = 0; i < threadPool.size(); ++i)
      PostQueuedCompletionStatus(iocpHandle, 0, 0, NULL);

    WaitForMultipleObjects(static_cast<DWORD>(threadPool.size()),
                           threadPool.data(), TRUE, INFINITE);

    for (size_t i = 0; i < threadPool.size(); ++i) CloseHandle(threadPool[i]);

    CloseHandle(iocpHandle);

    WSACleanup();
    std::cout << "IOCP server stopped." << std::endl;
  }
};

#endif  // _WIN32