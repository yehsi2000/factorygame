#ifndef SYSTEM_NETWORKSYSTEM_20COPY_
#define SYSTEM_NETWORKSYSTEM_20COPY_
#ifndef SYSTEM_NETWORKSYSTEM_
#define SYSTEM_NETWORKSYSTEM_

#include <memory>
#include <unordered_map>
#include <string>
#include <cstdint>

#include "Core/SystemContext.h"

class EventHandle;

class ServerNetworkSystem {
  AssetManager* assetManager;
  EventDispatcher* eventDispatcher;
  Registry* registry;
  CommandQueue* commandQueue;
  TimerManager* timerManager;
  ThreadSafeQueue<RecvPacket>* recvQueue; // Incoming packets
  ThreadSafeQueue<SendRequest>* sendQueue; // Outgoing packets (server-specific)
  World* world;
  Server* server;
  std::unordered_map<clientid_t, std::string>* clientNameMap;
  std::size_t playerSnapShotSize;

  float syncTimer;
  const float syncRate = 30.f; // send sync packet every 1/syncRate sec

 public:
  ServerNetworkSystem(const SystemContext& context);
  ~ServerNetworkSystem();
  void Update(float deltatime);
  void AddPlayerToMap(clientid_t clientId, std::string name);

 private:
  std::unique_ptr<EventHandle> sendChatHandle;
  void Unicast(uint64_t clientId, PacketPtr packet);
  void Broadcast(PacketPtr packet);
  void AddSyncPacket();
};

#endif /* SYSTEM_NETWORKSYSTEM_ */


#endif/* SYSTEM_NETWORKSYSTEM_20COPY_ */
