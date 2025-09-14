#ifndef CORE_SOCKET_
#define CORE_SOCKET_

#include <memory>
#include <cstddef>
#include <cstdint>
#include <string>

class SocketImpl;

/**
 * @brief Manages the client-side network communication.
 * @details This class provides a high-level interface for client socket
 * operations, abstracting the platform-specific implementation details. It
 * handles the lifecycle of the socket, including initialization, connection,
 * and data transfer.
 */
class Socket {
    std::unique_ptr<SocketImpl> pimpl;

public:
    Socket();
    ~Socket();
    
    Socket(Socket&&) noexcept;
    Socket& operator=(Socket&&) noexcept;

    bool Init();
    uint64_t Connect(std::string ip, int port);
    int Send(uint8_t* buffer, std::size_t size);
    int Receive(uint8_t* buffer, std::size_t size);
    void Close();


private:
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
};

#endif/* CORE_SOCKET_ */
