#ifndef SYSTEM_NETWORKSYSTEM_
#define SYSTEM_NETWORKSYSTEM_

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

#include "Core/SystemContext.h"

class EventHandle;

class ServerNetworkSystem {
  AssetManager* assetManager;
  EventDispatcher* eventDispatcher;
  Registry* registry;
  CommandQueue* commandQueue;
  TimerManager* timerManager;
  ThreadSafeQueue<RecvPacket>* recvQueue;  // Incoming packets
  ThreadSafeQueue<SendRequest>*
      sendQueue;  // Outgoing packets (server-specific)
  World* world;
  Server* server;
  std::unordered_map<clientid_t, std::string>* clientNameMap;

  std::size_t playerSnapShotSize;

  float syncTimer;

  ThreadSafeQueue<MoveApplied>* pendingMoves;

 public:
  ServerNetworkSystem(const SystemContext& context);
  ~ServerNetworkSystem();
  void Update(float deltatime);
  void AddPlayerToMap(clientid_t clientID, std::string name);

 private:
  std::unique_ptr<EventHandle> sendChatHandle;
  void Unicast(uint64_t clientID, PacketPtr packet);
  void Broadcast(PacketPtr packet);
  void SendSyncPacket();
  void ConnectSynHandler(const RecvPacket& recv, clientid_t clientID, const uint8_t* rp,
                         std::size_t packetSize);
  void ChatClientHandler(clientid_t clientID, const uint8_t* rp, std::size_t packetSize);
  void ClientMoveReqHandler(clientid_t clientID, const uint8_t* rp, std::size_t packetSize);
};

#endif /* SYSTEM_NETWORKSYSTEM_ */
