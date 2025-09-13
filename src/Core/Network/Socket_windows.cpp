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
#include <cstdlib>
#include <iostream>
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
      std::cerr << "Can't Initialize winsock" << std::endl;
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
      std::cerr << "getaddrinfo failed: " << gai_strerror(res) << std::endl;
      return 0;
    }

    for (addrIter = addrInfoList; addrIter != nullptr;
         addrIter = addrIter->ai_next) {
      connectSocket = socket(addrIter->ai_family, addrIter->ai_socktype,
                             addrIter->ai_protocol);
      if (connectSocket == INVALID_SOCKET) {
        std::cerr << "Error at socket() error code: " << WSAGetLastError() << std::endl;
        continue;
      }

      res =
          connect(connectSocket, addrIter->ai_addr, (int)addrIter->ai_addrlen);
      if (res == SOCKET_ERROR) {
        std::cerr << "Error at connect() error code: " << WSAGetLastError()
                  << std::endl;
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
      std::cerr << "Unable to connect to server after trying all addresses" << std::endl;
      return 0;
    }

    std::cout << "Connected to server" << std::endl;

    return static_cast<uint64_t>(connectSocket);
  }

  int Send(uint8_t* buffer, std::size_t size) override {
    int res = send(connectSocket, reinterpret_cast<char*>(buffer), size, 0);
    if (res == SOCKET_ERROR) {
      std::cerr << "Error at send() error code: " << WSAGetLastError() << std::endl;
      Close();
    }
    return res;
  }

  int Receive(uint8_t* buffer, std::size_t size) override {
    int res = recv(connectSocket, reinterpret_cast<char*>(buffer), size, 0);
    if (res == 0) {
      std::cout << "Connection closed" << std::endl;
    } else if (res == SOCKET_ERROR) {
      std::cerr << "Error at recv() error code: " << WSAGetLastError()
                << std::endl;
      Close();
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