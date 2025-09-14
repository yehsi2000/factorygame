#include "System/ServerNetworkSystem.h"

#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>

#include "Commands/PlayerDisconnectedCommnad.h"
#include "Commands/PlayerSpawnCommand.h"
#include "Components/InputStateComponent.h"
#include "Components/PlayerStateComponent.h"
#include "Components/SpriteComponent.h"
#include "Components/TransformComponent.h"
#include "Core/CommandQueue.h"
#include "Core/Event.h"
#include "Core/EventDispatcher.h"
#include "Core/Packet.h"
#include "Core/Server.h"
#include "Core/ThreadSafeQueue.h"
#include "Util/PacketUtil.h"


ServerNetworkSystem::ServerNetworkSystem(const SystemContext& context)
    : assetManager(context.assetManager),
      eventDispatcher(context.eventDispatcher),
      registry(context.registry),
      commandQueue(context.commandQueue),
      world(context.world),
      timerManager(context.timerManager),
      recvQueue(context.serverRecvQueue),  // Incoming packets (server-specific)
      sendQueue(context.serverSendQueue),  // Outgoing packets (server-specific)
      server(context.server),
      clientNameMap(context.clientNameMap),
      pendingMoves(context.pendingMoves),
      syncTimer(0.f),
      playerSnapShotSize(0) {
  // Subscribe chat event
  sendChatHandle =
      eventDispatcher->Subscribe<SendChatEvent>([this](SendChatEvent e) {
        Broadcast(util::ChatBroadcastPacket(e.message));
      });

  for (auto& [id, name] : *clientNameMap)
    playerSnapShotSize += sClientID + sizeof(uint8_t) + name.size();
}

void ServerNetworkSystem::ConnectSynHandler(const RecvPacket& recv,
                                            clientid_t clientID,
                                            const uint8_t* rp,
                                            std::size_t packetSize) {
  std::cout << "CONNECT_SYN from clientID: " << clientID << "\n";
  const uint8_t nameLen = *rp++;
  if (packetSize < (static_cast<size_t>(rp - recv.packet.get()) + nameLen)) {
    // invalid
    return;
  }
  constexpr size_t cap = NAME_MAX_LEN - 1;
  size_t copyLen = util::utf8_clamp_to_codepoint(rp, nameLen, cap);
  std::string name(reinterpret_cast<const char*>(rp), copyLen);

  // Generate character of connected client
  commandQueue->Enqueue(std::make_unique<PlayerSpawnCommand>(clientID, false));

  {  // Send CONNECT_ACK for connected client
    std::size_t totalPacketSize = sPacketHeader + sizeof(clientid_t) +
                                  sizeof(uint16_t) +
                                  playerSnapShotSize;  // header + playercnt

    std::unique_ptr<uint8_t[]> snapshotPacket =
        std::make_unique<uint8_t[]>(totalPacketSize);

    uint8_t* wp = snapshotPacket.get();

    util::WriteHeader(wp, PACKET::CONNECT_ACK, totalPacketSize);
    util::Write64BigEnd(wp, clientID);
    if (clientNameMap->size() != 0) {
      util::Write16BigEnd(wp, static_cast<uint16_t>(clientNameMap->size()));
      for (auto& [id, name] : *clientNameMap) {
        util::Write64BigEnd(wp, id);
        *wp++ = static_cast<uint8_t>(name.size());
        std::memcpy(wp, name.c_str(), name.size());

        wp += name.size();
      }
      Unicast(clientID, std::move(snapshotPacket));
    }
  }

  AddPlayerToMap(clientID, name);

  {  // BROADCAST PLAYER_CONNECTED TO ALL PLAYERS
    PacketPtr packet = std::make_unique<uint8_t[]>(
        sHeaderAndId + sizeof(uint8_t) + name.size());
    uint8_t* wp = packet.get();
    util::WriteHeader(wp, PACKET::PLAYER_CONNECTED_BROADCAST,
                      sHeaderAndId + sizeof(uint8_t) + name.size());
    util::Write64BigEnd(wp, clientID);
    *wp++ = static_cast<uint8_t>(name.size());
    std::memcpy(wp, name.c_str(), name.size());

    Broadcast(std::move(packet));
  }
}

void ServerNetworkSystem::ChatClientHandler(clientid_t clientID,
                                            const uint8_t* rp,
                                            std::size_t packetSize) {
  std::cout << "CHAT_CLIENT from clientID: " << clientID << "\n";
  // Broadcast chat to everyone
  const char* msgStart = reinterpret_cast<const char*>(rp);
  std::size_t msgSize = packetSize - sPacketHeader;
  std::shared_ptr<std::string> message =
      std::make_shared<std::string>(msgStart, msgSize);

  auto iter = clientNameMap->find(clientID);

  if (iter != clientNameMap->end()) {
    eventDispatcher->Publish(NewChatEvent(clientID, message));

    PacketPtr packet = util::ChatBroadcastPacket(message, clientID);
    Broadcast(std::move(packet));
  }
}

void ServerNetworkSystem::ClientMoveReqHandler(clientid_t clientID,
                                               const uint8_t* rp,
                                               std::size_t /*packetSize*/) {
  uint16_t seq = util::Read16BigEnd(rp);
  uint8_t inputBit = *rp++;

#ifdef PACKET_DEBUG
  std::cout << "CLIENT_MOVE_REQ from clientID: " << clientID
            << " seq=" << seq << " inputBit=" << static_cast<int>(inputBit)
            << "\n";
#endif

  EntityID e = world->GetPlayerByClientID(clientID);
  if (e == INVALID_ENTITY) return;

  if (!registry->HasComponent<InputStateComponent>(e)) {
    registry->EmplaceComponent<InputStateComponent>(e);
  }
  auto& inputState = registry->GetComponent<InputStateComponent>(e);
  inputState.inputBit = inputBit;
  // Only process newer inputs to avoid out-of-order execution
  if (util::seq_gt(seq, inputState.sequence)) {
    inputState.sequence = seq;
  }
}

void ServerNetworkSystem::Update(float deltatime) {
  // Process incoming packets
  RecvPacket recv;
  while (recvQueue->TryPop(recv)) {
    if (recv.packet == nullptr) {
      // Player Disconnected
      auto iter = clientNameMap->find(recv.senderClientId);
      if (iter != clientNameMap->end()) {
        std::string name = iter->second;
        clientNameMap->erase(iter);
        playerSnapShotSize -= sClientID + sizeof(uint8_t) + name.size();

        commandQueue->Enqueue(
            std::make_unique<PlayerDisconnectedCommand>(recv.senderClientId));

        // Broadcast PLAYER_DISCONNECTED to all players
        PacketPtr packet = std::make_unique<uint8_t[]>(sHeaderAndId);
        uint8_t* wp = packet.get();
        util::WriteHeader(wp, PACKET::PLAYER_DISCONNECTED_BROADCAST,
                          sHeaderAndId);
        util::Write64BigEnd(wp, recv.senderClientId);

        Broadcast(std::move(packet));
      }
      continue;
    }

    const uint8_t* rp = recv.packet.get();
    std::size_t packetSize;
    PACKET packetId;

    util::GetHeader(rp, packetId, packetSize);
    clientid_t clientID = recv.senderClientId;

    switch (packetId) {
      // TODO : add duplicate name check packet
      case CONNECT_SYN:
        ConnectSynHandler(recv, clientID, rp, packetSize);
        break;

      case CHAT_CLIENT:
        ChatClientHandler(clientID, rp, packetSize);
        break;

      case CLIENT_MOVE_REQ:
        ClientMoveReqHandler(clientID, rp, packetSize);
        break;
    }
  }

  // Periodic snapshot
  syncTimer += deltatime;
  if (syncTimer >= (1.f / syncRate)) {
    syncTimer -= (1.f / syncRate);
    SendSyncPacket();
  }

  // Send applied move result to requested client
  MoveApplied mv;
  while (pendingMoves->TryPop(mv)) {
    // Unicast immediate move result
    const std::size_t payloadSize = sizeof(uint16_t) + sizeof(float) * 2;
    const std::size_t totalSize = sPacketHeader + payloadSize;

    PacketPtr pkt = std::make_unique<uint8_t[]>(totalSize);
    uint8_t* p = pkt.get();

    util::WriteHeader(p, PACKET::CLIENT_MOVE_RES, totalSize);
    util::Write16BigEnd(p, mv.seq);
    util::WriteF32BigEnd(p, mv.x);
    util::WriteF32BigEnd(p, mv.y);

    Unicast(mv.clientID, std::move(pkt));
  }
}

void ServerNetworkSystem::AddPlayerToMap(clientid_t clientID,
                                         std::string name) {
  clientNameMap->emplace(clientID, name);
  playerSnapShotSize += sClientID + sizeof(uint8_t) + name.size();
}

void ServerNetworkSystem::SendSyncPacket() {
  struct Entry {
    clientid_t id;
    float x;
    float y;
    uint8_t facing;
  };

  std::vector<Entry> entries;

  for (EntityID player :
       registry->view<PlayerStateComponent, TransformComponent>()) {
    const auto& pc = registry->GetComponent<PlayerStateComponent>(player);
    const auto& t = registry->GetComponent<TransformComponent>(player);
    const auto& spr = registry->GetComponent<SpriteComponent>(player);
    uint8_t facing = spr.flip == SDL_FLIP_HORIZONTAL ? 1 : 0;

    entries.push_back(Entry{pc.clientID, t.position.x, t.position.y, facing});
  }

  const std::size_t packetSize =
      sPacketHeader + sizeof(uint16_t) +
      entries.size() * (sClientID + sizeof(float) * 2 + sizeof(uint8_t));

  PacketPtr packet = std::make_unique<uint8_t[]>(packetSize);
  uint8_t* wp = packet.get();

  util::WriteHeader(wp, PACKET::TRANSFORM_SNAPSHOT, packetSize);
  util::Write16BigEnd(wp, static_cast<uint16_t>(entries.size()));

  for (auto& e : entries) {
    util::Write64BigEnd(wp, e.id);
    util::WriteF32BigEnd(wp, e.x);
    util::WriteF32BigEnd(wp, e.y);
    *wp++ = e.facing;
  }
  Broadcast(std::move(packet));
}

void ServerNetworkSystem::Unicast(clientid_t clientID, PacketPtr packet) {
  SendRequest request;
  request.type = ESendType::UNICAST;
  request.targetClientId = clientID;
  request.packet = std::move(packet);
  sendQueue->Push(std::move(request));
  server->StartSend();
}

void ServerNetworkSystem::Broadcast(PacketPtr packet) {
  SendRequest request;
  request.type = ESendType::BROADCAST;
  request.targetClientId = 0;
  request.packet = std::move(packet);
  sendQueue->Push(std::move(request));
  server->StartSend();
}
ServerNetworkSystem::~ServerNetworkSystem() = default;