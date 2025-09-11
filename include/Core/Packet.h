#ifndef CORE_PACKET_
#define CORE_PACKET_

#include <cstdint>
#include <memory>

using PacketPtr = std::unique_ptr<char[]>;

enum PACKET { CHAT_CLIENT = 101, CHAT_BROADCAST = 102 };

/** Packet Description
 * CHAT_CLIENT :
 * PacketHeader header
 * uintptr_t clientSocketID
 * char message[header->packet_size - sizeof(PacketHeader) - sizeof(uintptr_t)]
 *
 *
 * CHAT_BROADCAST :
 * PacketHeader header
 * char message[header->packet_size - sizeof(PacketHeader)]
 */

struct PacketHeader {
  uint16_t packet_id;
  uint16_t packet_size;
};

struct Packet {
  PacketHeader header;
};

enum class ESendType {
  UNICAST,
  BROADCAST,
  SERVER,
};

struct SendRequest {
  ESendType type;
  uintptr_t targetClientId;  // positive int for UNICAST (0 for BROADCAST)
  PacketPtr packet;
};

#endif /* CORE_PACKET_ */
