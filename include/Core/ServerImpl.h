#ifndef CORE_SERVERIMPL_
#define CORE_SERVERIMPL_

#include <cstdint>
#include "Core/Packet.h"
#include "Core/ThreadSafeQueue.h"

/**
 * @brief Interface for the server implementation.
 * @details Defines the contract for platform-specific server implementations.
 * This allows the Server class to use the PIMPL idiom, separating the public
 * interface from the underlying implementation details and improving
- * compilation times.
 */
class ServerImpl {
public:
    virtual ~ServerImpl() = default;
    virtual bool Init(ThreadSafeQueue<RecvPacket>* recvQ, ThreadSafeQueue<SendRequest>* sendQ) = 0;
    virtual void StartSend() = 0;
    virtual void Start() = 0;
    virtual void Stop() = 0;
};

#endif/* CORE_SERVERIMPL_ */
