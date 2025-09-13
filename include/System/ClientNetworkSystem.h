#ifndef SYSTEM_CLIENTNETWORKSYSTEM_
#define SYSTEM_CLIENTNETWORKSYSTEM_

#include <memory>
#include <string>
#include <cstdint>
#include <unordered_map>

#include "Core/SystemContext.h"

class EventHandle;

class ClientNetworkSystem {
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
  const float moveReqRate = 30.f; // send packet every 1/syncRate sec

 public:
  ClientNetworkSystem(const SystemContext& context);
  ~ClientNetworkSystem();
  void Init(std::u8string playerName);
  void Update(float deltatime);
  

 private:
  std::unique_ptr<EventHandle> sendChatHandle;
  void SendMessage(std::shared_ptr<std::string> message);
  void SendMoveRequest(float deltaTime);
};

#endif/* SYSTEM_CLIENTNETWORKSYSTEM_ */
