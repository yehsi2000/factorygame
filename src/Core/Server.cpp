#include "Core/Server.h"

#include "Core/Packet.h"
#include "Core/ServerImpl.h"

// Include the platform-specific implementation files
#if defined(_WIN32)
#include "Network/Server_windows.cpp"
#elif defined(__linux__)
#include "Network/Server_linux.cpp"
#endif

Server::Server() {
#if defined(_WIN32)
  pimpl = std::make_unique<WindowsServerImpl>();
#elif defined(__linux__)
  pimpl = std::make_unique<LinuxServerImpl>();
#else
  // No implementation for unsupported platforms, pimpl will be null.
  // TODO : Create a default "Unsupported" implementation.
#endif
}

Server::~Server() = default;

Server::Server(Server&&) noexcept = default;
Server& Server::operator=(Server&&) noexcept = default;

bool Server::Init(ThreadSafeQueue<RecvPacket>* recvQ, ThreadSafeQueue<SendRequest>* sendQ) {
  if (pimpl)
    return pimpl->Init(recvQ, sendQ);
  else
    return false;
}

void Server::StartSend(){
  if (pimpl) pimpl->StartSend();
}

void Server::Start() {
  if (pimpl) pimpl->Start();
}

void Server::Stop() {
  if (pimpl) pimpl->Stop();
}
