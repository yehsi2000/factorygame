#ifndef SYSTEM_NETWORKSYSTEM_20COPY_
#define SYSTEM_NETWORKSYSTEM_20COPY_
#ifndef SYSTEM_NETWORKSYSTEM_
#define SYSTEM_NETWORKSYSTEM_

#include <memory>
#include <string>

#include "Core/SystemContext.h"

class EventHandle;

class ServerNetworkSystem {
  AssetManager* assetManager;
  EventDispatcher* eventDispatcher;
  Registry* registry;
  TimerManager* timerManager;
  ThreadSafeQueue<PacketPtr>* packetQueue;
  ThreadSafeQueue<SendRequest>* sendQueue;
  World* world;
  Server* server;

 public:
  ServerNetworkSystem(const SystemContext& context);
  ~ServerNetworkSystem();
  void Update(float deltatime);

 private:
  std::unique_ptr<EventHandle> sendChatHandle;
  void Unicast(uintptr_t clientId, PacketPtr packet);
  void Broadcast(PacketPtr packet);
};

#endif /* SYSTEM_NETWORKSYSTEM_ */


#endif/* SYSTEM_NETWORKSYSTEM_20COPY_ */
