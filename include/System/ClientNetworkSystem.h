#ifndef SYSTEM_CLIENTNETWORKSYSTEM_
#define SYSTEM_CLIENTNETWORKSYSTEM_

#include <cstdint>
#include <deque>
#include <memory>
#include <string>
#include <unordered_map>

#include "Core/SystemContext.h"
#include "Core/RingBuffer.h"

class EventHandle;
class NetPredictionComponent;
class MovementComponent;

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
  ThreadSafeQueue<PacketPtr>* sendQueue;  // Now queues PacketPtr directly
  World* world;
  Socket* connectionSocket;
  uint64_t myClientID;
  std::unordered_map<clientid_t, std::string>* clientNameMap;
  float moveReqTimer;
  std::string myName;

 public:
  ClientNetworkSystem(const SystemContext& context);
  ~ClientNetworkSystem();
  void Init(std::u8string playerName);
  void Update(float deltatime);

 private:
  std::unique_ptr<EventHandle> sendChatHandle;
  void ConnectAckHandler(const uint8_t* rp);
  void ChatBroadcastHandler(const uint8_t* rp, std::size_t packetSize);
  void TransformSnapshotHandler(const uint8_t* rp, double now);
  void ClientMoveResHandler(const uint8_t* rp);  // Server reconciliation

  void ApplyMovePrediction(NetPredictionComponent& pred,
                       const MovementComponent& move, uint8_t inputBit,
                       float deltaTime);
  void ApplyRemoteInterpolation(double now);
  void ApplyLocalSmoothing(float deltaTime);

  void SendMessage(std::shared_ptr<std::string> message);
  void SendMoveRequest(float deltaTime);

  // For client-side prediction and server reconciliation
  RingBuffer<InputCommand, 64> pendingInputQueue;
  static constexpr float kCatchUpSpeed = 20.f;
  static constexpr double kInterpolationDelay = 0.1;
  uint16_t inputSequenceNumber = 0;
};

#endif /* SYSTEM_CLIENTNETWORKSYSTEM_ */
