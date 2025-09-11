#ifndef CORE_SERVERIMPL_
#define CORE_SERVERIMPL_

#include <cstdint>
#include "Core/Packet.h"
#include "Core/ThreadSafeQueue.h"

class ServerImpl {
public:
    virtual ~ServerImpl() = default;
    virtual bool Init(ThreadSafeQueue<PacketPtr>* packQ, ThreadSafeQueue<SendRequest>* sendQ) = 0;
    virtual void StartSend() = 0;
    virtual void Start() = 0;
    virtual void Stop() = 0;
};

#endif/* CORE_SERVERIMPL_ */
