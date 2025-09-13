#include "Core/SocketImpl.h"

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <WS2tcpip.h>
#include <WinSock2.h>
#include <process.h>
#include <windows.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

class WindowsSocketImpl : public SocketImpl {
  SOCKET connectSocket = INVALID_SOCKET;
  WSABUF dataBuf;
  HANDLE plistenThreadHandle;
  struct addrinfo *addrInfoList = nullptr, *addrIter = nullptr, hints;
  std::string address;

  bool Init() override {
    WSADATA wsaData;
    int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (res != 0) {
      fprintf(stderr, "Can't Initialize winsock\n");
      return false;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    return true;
  }

  uint64_t Connect(std::string ip, int port) override {
    if (connectSocket != INVALID_SOCKET) {
      return 0;
    }

    int res = getaddrinfo(ip.c_str(), std::to_string(port).c_str(), &hints,
                          &addrInfoList);
    if (res != 0) {
      fprintf(stderr, "getaddrinfo failed: %s\n", gai_strerror(res));
      return 0;
    }

    for (addrIter = addrInfoList; addrIter != nullptr;
         addrIter = addrIter->ai_next) {
      connectSocket = socket(addrIter->ai_family, addrIter->ai_socktype,
                             addrIter->ai_protocol);
      if (connectSocket == INVALID_SOCKET) {
        fprintf(stderr, "Error at socket() error code: %d\n",
                WSAGetLastError());
        continue;
      }

      res =
          connect(connectSocket, addrIter->ai_addr, (int)addrIter->ai_addrlen);
      if (res == SOCKET_ERROR) {
        fprintf(stderr, "Error at connect() error code: %d\n",
                WSAGetLastError());
        closesocket(connectSocket);
        connectSocket = INVALID_SOCKET;
        continue;
      }
      break;
    }

    freeaddrinfo(addrInfoList);
    addrInfoList = nullptr;
    addrIter = nullptr;

    if (connectSocket == INVALID_SOCKET) {
      fprintf(stderr,
              "Unable to connect to server after trying all addresses\n");
      return 0;
    }

    printf("Connected to server\n");

    return static_cast<uint64_t>(connectSocket);
  }

  int Send(uint8_t* buffer, std::size_t size) override {
    int res = send(connectSocket, reinterpret_cast<char*>(buffer), size, 0);
    if (res == SOCKET_ERROR) {
      fprintf(stderr, "Error at send() error code: %d\n", WSAGetLastError());
      Close();
      return res;
    }
    printf("sent bytes : ");
    for (int i = 0; i < size; ++i) {
      printf("%x ", buffer[i]);
    }
    printf("\n");
    return res;
  }

  int Receive(uint8_t* buffer, std::size_t size) override {
    int res = recv(connectSocket, reinterpret_cast<char*>(buffer), size, 0);
    if (res > 0) {
      printf("received bytes : ");
      for (int i = 0; i < size; ++i) {
        printf("%x", buffer[i]);
      }
      printf("\n");
    } else if (res == 0) {
      printf("Connection closed\n");
    } else {
      fprintf(stderr, "Error at recv() error code: %d\n", WSAGetLastError());
    }
    return res;
  }

  void Close() override {
    if (plistenThreadHandle != nullptr) {
      if (connectSocket != INVALID_SOCKET) {
        closesocket(connectSocket);
        connectSocket = INVALID_SOCKET;
      }

      WaitForSingleObject(plistenThreadHandle, 3000);

      CloseHandle(plistenThreadHandle);
      plistenThreadHandle = nullptr;
    }
    connectSocket = INVALID_SOCKET;
    WSACleanup();
  }
};

#endif  // _WIN32