#ifndef CORE_SERVER_
#define CORE_SERVER_

#include <memory>
#include <cstdint>

#include "Core/Packet.h"
#include "Core/ThreadSafeQueue.h"

class ServerImpl;

/**
 * @brief Manages the server-side network communication.
 * @details This class provides a high-level interface for server operations,
 * abstracting the platform-specific implementation details. It handles the
 * lifecycle of the server, including initialization, starting, and stopping
 * the network threads.
 */
class Server {
    std::unique_ptr<ServerImpl> pimpl;

public:
    Server();
    ~Server();

    Server(Server&&) noexcept;
    Server& operator=(Server&&) noexcept;

    bool Init(ThreadSafeQueue<RecvPacket>* recvQ, ThreadSafeQueue<SendRequest>* sendQ);
    void StartSend();
    void Start();
    void Stop();

private:
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;
};

#endif/* CORE_SERVER_ */
