#ifndef UTIL_PACKETUTIL_
#define UTIL_PACKETUTIL_

#include <memory>
#include <string>

#include "Core/Packet.h"

namespace util {
static PacketPtr ChatBroadcastPacket(std::shared_ptr<std::string> message) {
  PacketPtr packet =
      std::make_unique<char[]>(message->size() + sizeof(PacketHeader));
  PacketHeader* packetHeader = reinterpret_cast<PacketHeader*>(packet.get());
  packetHeader->packet_id = CHAT_BROADCAST;
  packetHeader->packet_size = message->size() + sizeof(PacketHeader);
  std::memcpy(packet.get() + sizeof(PacketHeader), message->c_str(),
              message->size());
  return packet;
}
}  // namespace util

#endif /* UTIL_PACKETUTIL_ */
