#ifndef SYSTEM_NETWORKSYSTEM_
#define SYSTEM_NETWORKSYSTEM_

#include <memory>
#include <string>

#include "Core/SystemContext.h"

class EventHandle;

class ClientNetworkSystem {
  AssetManager* assetManager;
  EventDispatcher* eventDispatcher;
  Registry* registry;
  TimerManager* timerManager;
  ThreadSafeQueue<PacketPtr>* packetQueue;
  ThreadSafeQueue<SendRequest>* sendQueue;
  World* world;
  Socket* connectionSocket;
  uintptr_t clientID;

 public:
  ClientNetworkSystem(const SystemContext& context);
  ~ClientNetworkSystem();
  void Update(float deltatime);

 private:
  std::unique_ptr<EventHandle> sendChatHandle;
  void SendMessage(std::shared_ptr<std::string> message);
};

#endif /* SYSTEM_NETWORKSYSTEM_ */
