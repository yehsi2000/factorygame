#ifndef CORE_SOCKET_
#define CORE_SOCKET_

#include <memory>
#include <cstddef>
#include <cstdint>
#include <string>

class SocketImpl;

class Socket {
    std::unique_ptr<SocketImpl> pimpl;

public:
    Socket();
    ~Socket();
    
    Socket(Socket&&) noexcept;
    Socket& operator=(Socket&&) noexcept;

    bool Init();
    uintptr_t Connect(std::string ip, int port);
    int Send(char* buffer, std::size_t size);
    int Receive(char* buffer, std::size_t size);
    void Close();


private:
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
};

#endif/* CORE_SOCKET_ */
