#ifndef UTIL_PACKETUTIL_
#define UTIL_PACKETUTIL_

#include <memory>
#include <string>
#include <cstdint>
#include <cstddef>

#include "Core/Packet.h"

namespace util {
inline void Write16BigEnd(uint8_t*& p, uint16_t v) {
  *p++ = static_cast<uint8_t>((v >> 8) & 0xFF);
  *p++ = static_cast<uint8_t>(v & 0xFF);
}
inline void Write32BigEnd(uint8_t*& p, uint32_t v) {
  *p++ = static_cast<uint8_t>((v >> 24) & 0xFF);
  *p++ = static_cast<uint8_t>((v >> 16) & 0xFF);
  *p++ = static_cast<uint8_t>((v >> 8) & 0xFF);
  *p++ = static_cast<uint8_t>(v & 0xFF);
}
inline void Write64BigEnd(uint8_t*& p, uint64_t v) {
  *p++ = static_cast<uint8_t>((v >> 56) & 0xFF);
  *p++ = static_cast<uint8_t>((v >> 48) & 0xFF);
  *p++ = static_cast<uint8_t>((v >> 40) & 0xFF);
  *p++ = static_cast<uint8_t>((v >> 32) & 0xFF);
  *p++ = static_cast<uint8_t>((v >> 24) & 0xFF);
  *p++ = static_cast<uint8_t>((v >> 16) & 0xFF);
  *p++ = static_cast<uint8_t>((v >> 8) & 0xFF);
  *p++ = static_cast<uint8_t>(v & 0xFF);
}
inline uint16_t Read16BigEnd(uint8_t*& p) {
  uint16_t v =
      (static_cast<uint16_t>(p[0]) << 8) | (static_cast<uint16_t>(p[1]));
  p += 2;
  return v;
}
inline uint32_t Read32BigEnd(uint8_t*& p) {
  uint32_t v = (static_cast<uint32_t>(p[0]) << 24) |
               (static_cast<uint32_t>(p[1]) << 16) |
               (static_cast<uint32_t>(p[2]) << 8) |
               (static_cast<uint32_t>(p[3]));
  p += 4;
  return v;
}
inline uint64_t Read64BigEnd(uint8_t*& p) {
  uint64_t v = (static_cast<uint64_t>(p[0]) << 56) |
               (static_cast<uint64_t>(p[1]) << 48) |
               (static_cast<uint64_t>(p[2]) << 40) |
               (static_cast<uint64_t>(p[3]) << 32) |
               (static_cast<uint64_t>(p[4]) << 24) |
               (static_cast<uint64_t>(p[5]) << 16) |
               (static_cast<uint64_t>(p[6]) << 8) |
               (static_cast<uint64_t>(p[7]));
  p += 8;
  return v;
}

static void WriteHeader(uint8_t*& beginPtr, enum PACKET packetID,
                        std::size_t size) {
  Write16BigEnd(beginPtr, packetID);
  Write16BigEnd(beginPtr, size);
}

static void GetHeader(uint8_t* beginPtr, enum PACKET& packetID,
                      std::size_t& size) {
  packetID = static_cast<PACKET>(Read16BigEnd(beginPtr));
  size = Read16BigEnd(beginPtr);
}

static PacketPtr ChatBroadcastPacket(std::shared_ptr<std::string> message) {
  PacketPtr packet =
      std::make_unique<uint8_t[]>(message->size() + sizeof(PacketHeader));
  uint8_t* ptr = packet.get();
  WriteHeader(ptr, PACKET::CHAT_BROADCAST,
              message->size() + sizeof(PacketHeader));
  std::memcpy(packet.get() + sizeof(PacketHeader), message->c_str(),
              message->size());
  return packet;
}

}  // namespace util

#endif /* UTIL_PACKETUTIL_ */
