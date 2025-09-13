#include "System/ServerNetworkSystem.h"

#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>

#include "Commands/PlayerSpawnCommand.h"
#include "Components/PlayerStateComponent.h"
#include "Components/TransformComponent.h"
#include "Components/MoveIntentComponent.h"
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
void ServerNetworkSystem::Update(float deltatime) {
  syncTimer += deltatime;
  if (syncTimer >= (1.f / syncRate)) {
    syncTimer -= syncRate;
    AddSyncPacket();
  }

  RecvPacket recv;
  while (recvQueue->TryPop(recv)) {
    uint8_t* p = recv.packet.get();
    std::size_t packetSize;
    PACKET packetId;

    util::GetHeader(p, packetId, packetSize);
    clientid_t clientId = recv.senderClientId;

    switch (packetId) {
      // TODO : add duplicate name check packet
      case CONNECT_SYN: {
        const uint8_t nameLen = *p++;
        if (packetSize <
            (static_cast<size_t>(p - recv.packet.get()) + nameLen)) {
          // invalid
          return;
        }
        constexpr size_t cap = NAME_MAX_LEN - 1;
        size_t copyLen = util::utf8_clamp_to_codepoint(p, nameLen, cap);
        std::string name(reinterpret_cast<const char*>(p), copyLen);

        // Generate character of connected client
        commandQueue->Enqueue(std::make_unique<PlayerSpawnCommand>(clientId, false));

        {  // Send CONNECT_ACK for connected client
          std::size_t totalPacketSize =
              sPacketHeader + sizeof(clientid_t) + sizeof(uint16_t) +
              playerSnapShotSize;  // header + playercnt

          std::unique_ptr<uint8_t[]> snapshotPacket =
              std::make_unique<uint8_t[]>(totalPacketSize);

          uint8_t* p = snapshotPacket.get();

          util::WriteHeader(p, PACKET::CONNECT_ACK, totalPacketSize);
          util::Write64BigEnd(p, clientId);
          if (clientNameMap->size() != 0) {
            util::Write16BigEnd(p, static_cast<uint16_t>(clientNameMap->size()));
            for (auto& [id, name] : *clientNameMap) {
              util::Write64BigEnd(p, id);
              *p++ = static_cast<uint8_t>(name.size());
              std::memcpy(p, name.c_str(), name.size());

              p += name.size();
            }
            Unicast(clientId, std::move(snapshotPacket));
          }
        }

        AddPlayerToMap(clientId, name);

        {  // BROADCAST PLAYER_CONNECTED TO ALL PLAYERS
          PacketPtr packet = std::make_unique<uint8_t[]>(
              sHeaderAndId + sizeof(uint8_t) + name.size());
          uint8_t* p = packet.get();
          util::WriteHeader(p, PACKET::PLAYER_CONNECTED_BROADCAST,
                            sHeaderAndId + sizeof(uint8_t) + name.size());
          util::Write64BigEnd(p, clientId);
          *p++ = static_cast<uint8_t>(name.size());
          std::memcpy(p, name.c_str(), name.size());

          Broadcast(std::move(packet));
        }

        break;
      }

      case CHAT_CLIENT: {
        // Broadcast chat to everyone
        char* msgStart = reinterpret_cast<char*>(p);
        std::size_t msgSize = packetSize - sHeaderAndId;
        std::shared_ptr<std::string> message =
            std::make_shared<std::string>(msgStart, msgSize);

        auto iter = clientNameMap->find(clientId);

        if (iter != clientNameMap->end()) {
          eventDispatcher->Publish(NewChatEvent(iter->second, message));

          PacketPtr packet = util::ChatBroadcastPacket(message, clientId);
          Broadcast(std::move(packet));
        }

        break;
      }

      case CLIENT_MOVE_REQ: {
        clientid_t clientId = util::Read64BigEnd(p);
        uint16_t seq = util::Read16BigEnd(p);
        uint8_t inputBit = *p++;
        float dt = util::ReadF32BigEnd(p);
        auto entity = world->GetEntityByClientID(clientId);
        if (entity != INVALID_ENTITY) {
          auto& intent = registry->GetComponent<MoveIntentComponent>(entity);
          intent.seq = seq;
          intent.inputBit = inputBit;
          intent.deltaTime = dt;
          intent.hasNew = true;
        }
        break;
      }
    }
  }
}

void ServerNetworkSystem::AddPlayerToMap(clientid_t clientId,
                                         std::string name) {
  clientNameMap->emplace(clientId, name);
  playerSnapShotSize += sClientID + sizeof(uint8_t) + name.size();
}


void ServerNetworkSystem::AddSyncPacket() {
  std::vector<std::tuple<clientid_t, float, float, uint8_t>> entries;
  for (EntityID e : registry->view<PlayerStateComponent, TransformComponent>()) {
    auto& pc = registry->GetComponent<PlayerStateComponent>(e);
    auto& t = registry->GetComponent<TransformComponent>(e);
    uint8_t facing = 1; // default right
    entries.emplace_back(pc.clientID, t.position.x, t.position.y, facing);
  }
  const std::size_t packetSize = sPacketHeader + sizeof(uint16_t) + entries.size() * (sClientID + sizeof(float)*2 + sizeof(uint8_t));
  PacketPtr pkt = std::make_unique<uint8_t[]>(packetSize);
  uint8_t* p = pkt.get();
  util::WriteHeader(p, PACKET::TRANSFORM_SNAPSHOT, packetSize);
  util::Write16BigEnd(p, static_cast<uint16_t>(entries.size()));
  
  for (auto& [id, x, y, facing] : entries) {
    util::Write64BigEnd(p, id);
    util::WriteF32BigEnd(p, x);
    util::WriteF32BigEnd(p, y);
    *p++ = facing;
  }
  Broadcast(std::move(pkt));
}

void ServerNetworkSystem::Unicast(clientid_t clientId, PacketPtr packet) {
  SendRequest request;
  request.type = ESendType::UNICAST;
  request.targetClientId = clientId;
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