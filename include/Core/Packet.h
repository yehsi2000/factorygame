#ifndef CORE_PACKET_
#define CORE_PACKET_

#include <cstdint>
#include <memory>

using PacketPtr = std::unique_ptr<uint8_t[]>;
using clientid_t = uint64_t;

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
  REQUEST,
};

struct SendRequest {
  ESendType type;
  clientid_t targetClientId;  // positive int for UNICAST (0 for BROADCAST)
  PacketPtr packet;
};

enum PACKET {
  /**
   * PLAYER_CONNECTED
   *
   * --- Payload ---
   * clientid_t : connected client SocketID
   */
  PLAYER_CONNECTED = 100,

  /**
   * CHAT_CLIENT :
   *
   * --- Payload ---
   * clientid_t : chatting clientSocketID
   * char[header->packet_size - (sPacketHeader + sClientID)] : message
   */
  CHAT_CLIENT = 101,

  /**
   * CHAT_BROADCAST :
   *
   * --- Payload ---
   * char[header->packet_size - sPacketHeader] : message
   */
  CHAT_BROADCAST = 102,

  /**
   * CLIENT_MOVE_REQ :
   *
   * --- Payload ---
   * uint8_t :    input_bit
   * float :      movement_normal_vector
   * clientid_t : requesting clientsocketID
   */
  CLIENT_MOVE_REQ = 103,

  /**
   * PLAYER_POS_BROADCAST :
   *
   * --- Payload ---
   * float :      x position
   * float :      y position
   * clientid_t : player's clientsocketID
   */
  PLAYER_POS_BROADCAST = 104,

  /**
   * PLAYER_LIST_SNAPSHOT :
   *
   * --- Payload ---
   * uint16_t : player_count
   *
   * Repeated For player_count
   * ---------------------------------
   * clientid_t player_id
   * uint8_t name_len
   * uint_t name[name_len] (UTF-8 without \0)
   * ---------------------------------
   */
  PLAYER_LIST_SNAPSHOT = 105
};

constexpr std::size_t sPacketHeader = sizeof(PacketHeader);
constexpr std::size_t sClientID = sizeof(clientid_t);
constexpr std::size_t sHeaderAndId = sPacketHeader + sClientID;

enum class EPlayerInput : uint8_t {
  UP = 1 << 0,     // 0000 0001
  DOWN = 1 << 1,   // 0000 0010
  LEFT = 1 << 2,   // 0000 0100
  RIGHT = 1 << 3,  // 0000 1000
};

#endif /* CORE_PACKET_ */
