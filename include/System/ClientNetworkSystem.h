#ifndef SYSTEM_CLIENTNETWORKSYSTEM_
#define SYSTEM_CLIENTNETWORKSYSTEM_

#include <memory>
#include <string>
#include <cstdint>
#include <deque>
#include <unordered_map>

#include "Core/SystemContext.h"

class EventHandle;

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
  ThreadSafeQueue<PacketPtr>* sendQueue; // Now queues PacketPtr directly
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
  void ConnectAckHandler(const uint8_t* rp, std::size_t packetSize);
  void ChatBroadcastHandler(const uint8_t* rp, std::size_t packetSize);
  void TransformSnapshotHandler(const uint8_t* rp, std::size_t packetSize);
  void ClientMoveResHandler(const uint8_t* rp,
                            std::size_t packetSize);  // Server reconciliation

  void ApplyRemoteInterpolation();
  void ApplyLocalSmoothing(float deltaTime);

  void SendMessage(std::shared_ptr<std::string> message);
  void SendMoveRequest(float deltaTime);

  // For client-side prediction and server reconciliation
  std::deque<InputCommand> m_pendingInputs;
  uint16_t m_inputSequenceNumber = 0;
};

#endif/* SYSTEM_CLIENTNETWORKSYSTEM_ */
