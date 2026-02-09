#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

#include "Core/SystemContext.h"
#include "DataStruct/RingBuffer.h"

class EventHandle;
class NetPredictionComponent;
class MovementComponent;

/**
 * @brief Manages client-side network communication, prediction, and
 * interpolation.
 */
class ClientNetworkSystem {
  struct InputCommand {
    uint16_t sequence;
    uint8_t inputBit;
    float predX;
    float predY;
    float deltaTime;
  };

  AssetManager* assetManager;
  EventDispatcher* eventDispatcher;
  CommandQueue* commandQueue;
  Registry* registry;
  InputManager* inputManager;
  TimerManager* timerManager;
  ThreadSafeQueue<PacketPtr>* recvQueue;
  ThreadSafeQueue<PacketPtr>* sendQueue;
  World* world;
  Socket* connectionSocket;

  uint64_t myClientID;
  std::unordered_map<clientid_t, std::string>* clientNameMap;
  float moveReqTimer;
  std::string myName;

 public:
  /**
   * @brief Constructs the ClientNetworkSystem with the given context.
   * @param context The system context containing shared resources.
   */
  explicit ClientNetworkSystem(const SystemContext& context);

  /**
   * @brief Default destructor.
   */
  ~ClientNetworkSystem();

  /**
   * @brief Initializes the connection to the server with the player's name.
   * @param playerName The UTF-8 string containing the player's name.
   */
  void Init(const std::u8string& playerName);

  /**
   * @brief Updates the network system, processing packets and applying
   * synchronization.
   * @param deltatime Time elapsed since the last update in seconds.
   */
  void Update(float deltatime);

 private:
  std::unique_ptr<EventHandle> sendChatHandle;

  /**
   * @brief Processes an individual packet received from the server.
   * @param packet The packet to process.
   * @param now Current steady clock time in seconds.
   */
  void ProcessPacket(const PacketPtr& packet, double now);

  /**
   * @brief Handles the connection acknowledgment from the server.
   * @param rp Pointer to the payload of the packet.
   */
  void ConnectAckHandler(const uint8_t* rp);

  /**
   * @brief Handles information about a new player connecting to the server.
   * @param rp Pointer to the payload of the packet.
   */
  void PlayerConnectHandler(const uint8_t* rp);

  /**
   * @brief Handles incoming chat messages broadcast from the server.
   * @param rp Pointer to the payload of the packet.
   * @param packetSize Total size of the packet.
   */
  void ChatBroadcastHandler(const uint8_t* rp, std::size_t packetSize);

  /**
   * @brief Handles snapshots of all entities' transforms from the server.
   * @param rp Pointer to the payload of the packet.
   * @param now Current steady clock time in seconds.
   */
  void TransformSnapshotHandler(const uint8_t* rp, double now);

  /**
   * @brief Handles the response to a move request, performing reconciliation if
   * needed.
   * @param rp Pointer to the payload of the packet.
   */
  void ClientMoveResHandler(const uint8_t* rp);

  /**
   * @brief Applies movement prediction based on input and delta time.
   * @param pred The prediction component to update.
   * @param move The movement constants for the entity.
   * @param inputBit Bitmask of active player inputs.
   * @param deltaTime Time step for the prediction.
   */
  void ApplyMovePrediction(NetPredictionComponent& pred,
                           const MovementComponent& move, uint8_t inputBit,
                           float deltaTime);
  /**
   * @brief Interpolates remote entities' transforms based on received
   * snapshots.
   * @param now Current steady clock time in seconds.
   */
  void ApplyRemoteInterpolation(double now);

  /**
   * @brief Smoothly interpolates the local player's visual position to the
   * predicted position.
   * @param deltaTime Time elapsed since the last update in seconds.
   *
   */
  void ApplyLocalSmoothing(float deltaTime);

  /**
   * @brief Sends a chat message to the server.
   * @param message Shared pointer to the string message.
   */
  void SendMessage(const std::shared_ptr<std::string>& message);

  /**
   * @brief Sends a movement request to the server based on current input.
   * @param deltaTime Time step for which the move is requested.
   */
  void SendMoveRequest(float deltaTime);

  RingBuffer<InputCommand, 64> pendingInputQueue;
  static constexpr float catchUpSpeed = 20.f;
  static constexpr double interpolationDelay = 0.1;
  uint16_t inputSequenceNumber = 0;
};
