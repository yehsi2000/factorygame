#ifndef CORE_SOCKETIMPL_
#define CORE_SOCKETIMPL_

#include <string>
#include <cstddef>
#include <cstdint>

class SocketImpl {
public:
    virtual ~SocketImpl() = default;
    virtual bool Init() = 0;
    virtual uint64_t Connect(std::string ip, int port) = 0;
    virtual int Send(uint8_t* buffer, std::size_t size) = 0;
    virtual int Receive(uint8_t* buffer, std::size_t size) = 0;
    virtual void Close() = 0;
};

#endif/* CORE_SOCKETIMPL_ */
