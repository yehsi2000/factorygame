#include "Core/Server.h"

#include "Core/ServerImpl.h"


// Include the platform-specific implementation files
#if defined(_WIN32)
#include "Server/Server_windows.cpp"
#elif defined(__linux__)
#include "Server/Server_linux.cpp"
#endif

// --- Server class implementation ---

Server::Server() {
#if defined(_WIN32)
  pimpl = std::make_unique<WindowsServerImpl>();
#elif defined(__linux__)
  pimpl = std::make_unique<LinuxServerImpl>();
#else
  // No implementation for unsupported platforms, pimpl will be null.
  // Alternatively, create a default "Unsupported" implementation.
#endif
}

// The destructor needs to be defined here where ServerImpl is a complete type.
Server::~Server() = default;

// Move semantics must also be defined here.
Server::Server(Server&&) noexcept = default;
Server& Server::operator=(Server&&) noexcept = default;

bool Server::Init() {
  if (pimpl) {
    return pimpl->Init();
  }
}

void Server::Start() {
  if (pimpl) {
    pimpl->Start();
  }
}

void Server::Stop() {
  if (pimpl) {
    pimpl->Stop();
  }
}
