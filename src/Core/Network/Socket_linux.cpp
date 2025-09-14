#include "Core/SocketImpl.h"

#ifdef __linux__

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

class LinuxSocketImpl : public SocketImpl {
  bool Init() override { return true; }

  uint64_t Connect(std::string ip, int port) override { return 0; }

  int Send(uint8_t* buffer, std::size_t size) override { return 0; }

  int Receive(uint8_t* buffer, std::size_t size) override { return 0; }

  void Close() override {};
};
#endif  // __linux__
