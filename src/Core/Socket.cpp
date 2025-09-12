#include "Core/Socket.h"

#include "Core/SocketImpl.h"

// Include the platform-specific implementation files
#if defined(_WIN32)
#include "Network/Socket_windows.cpp"
#elif defined(__linux__)
#include "Network/Socket_linux.cpp"
#endif

Socket::Socket() {
#if defined(_WIN32)
  pimpl = std::make_unique<WindowsSocketImpl>();
#elif defined(__linux__)
  pimpl = std::make_unique<LinuxSocketImpl>();
#else
  // No implementation for unsupported platforms, pimpl will be null.
  // TODO : Create a default "Unsupported" implementation.
#endif
}

Socket::~Socket() = default;

Socket::Socket(Socket&&) noexcept = default;
Socket& Socket::operator=(Socket&&) noexcept = default;

bool Socket::Init() {
  if (pimpl) return pimpl->Init();
  return false;
}

uint64_t Socket::Connect(std::string ip, int port) {
  if (pimpl) return pimpl->Connect(ip, port);
  return 0;
}

int Socket::Send(uint8_t* buffer, std::size_t size) {
  if (pimpl) return pimpl->Send(buffer, size);
  return 0;
}

int Socket::Receive(uint8_t* buffer, std::size_t size) {
  if (pimpl) return pimpl->Receive(buffer, size);
  return 0;
}

void Socket::Close() {
  if (pimpl) pimpl->Close();
}