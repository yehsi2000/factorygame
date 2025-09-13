#ifndef UTIL_PACKETUTIL_
#define UTIL_PACKETUTIL_

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>

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
inline uint16_t Read16BigEnd(const uint8_t*& p) {
  uint16_t v =
      (static_cast<uint16_t>(p[0]) << 8) | (static_cast<uint16_t>(p[1]));
  p += 2;
  return v;
}
inline uint32_t Read32BigEnd(const uint8_t*& p) {
  uint32_t v = (static_cast<uint32_t>(p[0]) << 24) |
               (static_cast<uint32_t>(p[1]) << 16) |
               (static_cast<uint32_t>(p[2]) << 8) |
               (static_cast<uint32_t>(p[3]));
  p += 4;
  return v;
}
inline uint64_t Read64BigEnd(const uint8_t*& p) {
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

inline void WriteF32BigEnd(uint8_t*& p, float f) {
  static_assert(sizeof(float) == 4, "float must be 32-bit");
  uint32_t bits;
  std::memcpy(&bits, &f, sizeof(bits));
  Write32BigEnd(p, bits);
}
inline float ReadF32BigEnd(const uint8_t*& p) {
  uint32_t bits = Read32BigEnd(p);
  float f;
  std::memcpy(&f, &bits, sizeof(f));
  return f;
}

inline size_t utf8_clamp_to_codepoint(const uint8_t* s, size_t len,
                                      size_t maxBytes) {
  size_t i = 0;
  while (i < len && i < maxBytes) {
    uint8_t c = s[i];
    size_t n = 1;
    if ((c & 0x80) == 0x00)  // 1byte header
      n = 1;
    else if ((c & 0xE0) == 0xC0)  // 2byte header
      n = 2;
    else if ((c & 0xF0) == 0xE0)  // 3byte header
      n = 3;
    else if ((c & 0xF8) == 0xF0)  // 4byte header
      n = 4;
    else
      break;  // invalid lead byte

    if (i + n > maxBytes) break;      // would split codepoint (by capacity)
    if (i + n > len) return i;        // truncated payload
    for (size_t k = 1; k < n; ++k) {  // validate continuation
      if ((s[i + k] & 0xC0) != 0x80) return i;
    }
    i += n;
  }
  return i;
}

static void WriteHeader(uint8_t*& beginPtr, enum PACKET packetID,
                        std::size_t size) {
  Write16BigEnd(beginPtr, static_cast<uint16_t>(packetID));
  Write16BigEnd(beginPtr, static_cast<uint16_t>(size));
}

static void GetHeader(uint8_t*& beginPtr, enum PACKET& packetID,
                      std::size_t& size) {
  packetID = static_cast<PACKET>(Read16BigEnd(beginPtr));
  size = Read16BigEnd(beginPtr);
}

static PacketPtr ChatBroadcastPacket(std::shared_ptr<std::string> message, clientid_t senderId) {
  const std::size_t payload = sizeof(clientid_t) + message->size();
  const std::size_t total = sPacketHeader + payload;
  PacketPtr packet = std::make_unique<uint8_t[]>(total);
  uint8_t* p = packet.get();
  WriteHeader(p, CHAT_BROADCAST, total);
  Write64BigEnd(p, senderId);
  std::memcpy(p, message->c_str(), message->size());
  return packet;
}

static PacketPtr ChatBroadcastPacket(std::shared_ptr<std::string> message) {
  return ChatBroadcastPacket(std::move(message), 0);
}

}  // namespace util

#endif /* UTIL_PACKETUTIL_ */
