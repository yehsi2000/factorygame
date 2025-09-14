#ifndef CORE_PACKET_
#define CORE_PACKET_

#include <cstdint>
#include <memory>

using PacketPtr = std::unique_ptr<uint8_t[]>;
using clientid_t = uint64_t;

// run sync between server-client "syncRate" times per second
constexpr float syncRate = 30.f;
constexpr float syncDelta = 1.f / syncRate;

#pragma pack(push, 1)
/**
 * @brief The header for all network packets.
 * @details Contains the packet's type identifier and its total size. This
 * information is used to route and deserialize the packet on the receiving
 * end.
 */
struct PacketHeader {
  uint16_t packet_id;
  uint16_t packet_size;
};

/**
 * @brief The base struct for all network packets.
 * @details All specific packet types should inherit from this struct to ensure
 * they include the standard packet header.
 */
struct Packet {
  PacketHeader header;
};
#pragma pack(pop)

/**
 * @brief Specifies the destination for an outgoing packet.
 */
enum class ESendType {
  UNICAST,
  BROADCAST,
};

/**
 * @brief A request to send a packet over the network.
 * @details This struct is used by the server to manage outgoing packets. It
 * includes the packet data, its destination type, and the target client ID if

 * it's a unicast message.
 */
struct SendRequest {
  ESendType type;
  clientid_t targetClientId;  // positive int for UNICAST (0 for BROADCAST)
  PacketPtr packet;
};

/**
 * @brief A packet received from the network.
 * @details This struct is used to store incoming packets. It includes the
 * packet data and the ID of the client who sent it.
 */
struct RecvPacket {
  clientid_t senderClientId;
  PacketPtr packet;
};

/**
 * @brief Represents a server-validated player movement.
 * @details This struct is used to communicate the result of a processed move
 * request back to the relevant systems.
 */
struct MoveApplied {
  clientid_t clientID;
  uint16_t seq;
  float x, y;
};

/**
 * @brief Defines the unique identifiers for each packet type.
 */
enum PACKET {
  /**
   * CONNECT_SYN
   *
   * --- Payload ---
   * uint8_t :    name_len
   * char :       name[name_len] (UTF-8 without \0)
   */
  CONNECT_SYN = 100,
  /**
   * CONNECT_ACK :
   *
   * --- Payload ---
   * clientid_t : fresh allocated clientID for syn sender
   * uint16_t : player_cnt
   *
   * [Repeated for player_cnt]
   * ---------------------------------
   * clientid_t : player_id
   * uint8_t :    name_len
   * char :     name[name_len] (UTF-8 without \0)
   * ---------------------------------
   */
  CONNECT_ACK,

  /**
   * PLAYER_CONNECTED_BROADCAST
   *
   * --- Payload ---
   * clientid_t : connected clientID
   * uint8_t :    name_len
   * char :     name[name_len] (UTF-8 without \0)
   */
  PLAYER_CONNECTED_BROADCAST,

  /**
   * CHAT_CLIENT :
   *
   * --- Payload ---
   * char[packet_size - sPacketHeader] : message
   */
  CHAT_CLIENT,

  /**
   * CHAT_BROADCAST :
   *
   * --- Payload ---
   * clientid_t : clientID of sent chat;
   * char[packet_size - sHeaderAndId] : message
   */
  CHAT_BROADCAST,

  /**
   * CLIENT_MOVE_REQ :
   *
   * --- Payload ---
   * uint16_t :   input_sequence_num
   * uint8_t :    input_bit                // used by server
   */
  CLIENT_MOVE_REQ,

  /**
   * CLIENT_MOVE_RES :
   *
   * --- Payload ---
   * uint16_t : last_acked_input_sequence_num
   * float :    posX
   * float :    posY
   */
  CLIENT_MOVE_RES,

  /**
   * TRANSFORM_SNAPSHOT :
   *
   * --- Payload ---
   * uint16_t : player_cnt
   *
   * [Repeated for player_cnt]
   * ---------------------------------
   * clientid_t : player_id
   * float : posX
   * float : posY
   * uint8_t : 0 = SDL_FLIP_NONE, 1 = SDL_FLIP_HORIZONTAL
   * ---------------------------------
   */
  TRANSFORM_SNAPSHOT,

  /**
   * PLAYER_DISCONNECTED_BROADCAST :
   *
   * --- Payload ---
   * clientid_t : disconnected clientID
   */
  PLAYER_DISCONNECTED_BROADCAST,
};

constexpr uint8_t NAME_MAX_LEN = 64;
constexpr std::size_t sPacketHeader = sizeof(PacketHeader);
constexpr std::size_t sClientID = sizeof(clientid_t);
constexpr std::size_t sHeaderAndId = sPacketHeader + sClientID;

/**
 * @brief Bitmask for player input states.
 */
enum class EPlayerInput : uint8_t {
  UP = 1 << 0,     // 0000 0001
  DOWN = 1 << 1,   // 0000 0010
  LEFT = 1 << 2,   // 0000 0100
  RIGHT = 1 << 3,  // 0000 1000
};

#endif/* CORE_PACKET_ */
