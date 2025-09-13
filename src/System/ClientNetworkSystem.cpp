#include "System/ClientNetworkSystem.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>

#include "Commands/PlayerSpawnCommand.h"
#include "Components/AnimationComponent.h"
#include "Components/LocalPlayerComponent.h"
#include "Components/MovementComponent.h"
#include "Components/NetPredictionComponent.h"
#include "Components/PlayerStateComponent.h"
#include "Components/SpriteComponent.h"
#include "Components/TransformComponent.h"
#include "Core/CommandQueue.h"
#include "Core/Event.h"
#include "Core/EventDispatcher.h"
#include "Core/InputManager.h"
#include "Core/Packet.h"
#include "Core/Socket.h"
#include "Util/AnimUtil.h"
#include "Util/PacketUtil.h"

ClientNetworkSystem::ClientNetworkSystem(const SystemContext& context)
    : assetManager(context.assetManager),
      eventDispatcher(context.eventDispatcher),
      registry(context.registry),
      commandQueue(context.commandQueue),
      world(context.world),
      inputManager(context.inputManager),
      timerManager(context.timerManager),
      recvQueue(context.clientRecvQueue),  // Incoming packets (client-specific)
      sendQueue(context.clientSendQueue),  // Outgoing packets (client-specific)
      connectionSocket(context.socket),
      clientNameMap(context.clientNameMap),
      myClientID(-1) {
  sendChatHandle = eventDispatcher->Subscribe<SendChatEvent>(
      [this](SendChatEvent e) { SendMessage(e.message); });
}

void ClientNetworkSystem::Init(std::u8string playerName) {
  const uint8_t nameSize =
      std::min(static_cast<uint8_t>(playerName.size()), NAME_MAX_LEN);

  const size_t packetSize = sPacketHeader + sizeof(uint8_t) + nameSize;

  PacketPtr packet = std::make_unique<uint8_t[]>(packetSize);

  uint8_t* p = packet.get();

  util::WriteHeader(p, PACKET::CONNECT_SYN, packetSize);
  *p++ = nameSize;
  std::memcpy(p, playerName.c_str(), nameSize);
  connectionSocket->Send(packet.get(), packetSize);
}

void ClientNetworkSystem::Update(float deltatime) {
  PacketPtr packet;
  while (recvQueue->TryPop(packet)) {
    uint8_t* p = packet.get();
    std::size_t packetSize;
    PACKET packetId;
    util::GetHeader(p, packetId, packetSize);
    switch (packetId) {
      case CONNECT_ACK: {
        myClientID = util::Read64BigEnd(p);

        uint16_t playerCnt = util::Read16BigEnd(p);

        for (int i = 0; i < playerCnt; ++i) {
          clientid_t id = util::Read64BigEnd(p);

          const uint8_t nameLen = *p++;
          constexpr size_t cap = NAME_MAX_LEN - 1;

          size_t copyLen = util::utf8_clamp_to_codepoint(p, nameLen, cap);
          std::string name(reinterpret_cast<const char*>(p), copyLen);

          p += name.size();

          clientNameMap->emplace(id, name);
          commandQueue->Enqueue(
              std::make_unique<PlayerSpawnCommand>(id, false));
        }
        if (world->GetLocalPlayer() == INVALID_ENTITY)
          world->GeneratePlayer(myClientID, {0.f, 0.f}, true);
        break;
      }

      case CHAT_BROADCAST: {
        clientid_t senderClientId = util::Read64BigEnd(p);
        if (senderClientId == myClientID) break;
        auto iter = clientNameMap->find(senderClientId);
        if (iter != clientNameMap->end()) {
          eventDispatcher->Publish(NewChatEvent(
              iter->second,
              std::make_shared<std::string>(reinterpret_cast<char*>(p),
                                            packetSize - sHeaderAndId)));
        }
        break;
      }

      case TRANSFORM_SNAPSHOT: {
        uint16_t count = util::Read16BigEnd(p);
        for (uint16_t i = 0; i < count; ++i) {
          clientid_t id = util::Read64BigEnd(p);
          float posX = util::ReadF32BigEnd(p);
          float posY = util::ReadF32BigEnd(p);
          uint8_t facing = *p++;

          // Find entity with matching clientID
          for (EntityID e :
               registry->view<PlayerStateComponent, TransformComponent>()) {
            auto& pc = registry->GetComponent<PlayerStateComponent>(e);
            if (pc.clientID != id) continue;
            auto& t = registry->GetComponent<TransformComponent>(e);
            t.position.x = posX;
            t.position.y = posY;
            if (registry->HasComponent<SpriteComponent>(e)) {
              auto& spr = registry->GetComponent<SpriteComponent>(e);
              spr.flip = (facing == 0) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
            }
            break;
          }
        }
        break;
      }
    }
  }

  PacketPtr outgoingPacket;
  while (sendQueue->TryPop(outgoingPacket)) {
    PacketHeader* p = reinterpret_cast<PacketHeader*>(outgoingPacket.get());
    connectionSocket->Send(outgoingPacket.get(), p->packet_size);
  }

  moveReqTimer += deltatime;
  if (moveReqTimer >= (1.f / moveReqRate)) {
    SendMoveRequest(moveReqTimer);
    moveReqTimer -= moveReqRate;
  }
}

void ClientNetworkSystem::SendMoveRequest(float deltaTime) {
  EntityID player = world->GetLocalPlayer();
  auto& playerAnimComp = registry->GetComponent<AnimationComponent>(player);
  auto& playerStateComp = registry->GetComponent<PlayerStateComponent>(player);
  int ix = inputManager->GetXAxis();
  int iy = inputManager->GetYAxis();
  if (ix == 0.f && iy == 0.f) {
    return;
  }

  const MovementComponent& move =
      registry->GetComponent<MovementComponent>(player);
  TransformComponent& trans =
      registry->GetComponent<TransformComponent>(player);
  auto& playerSpriteComp = registry->GetComponent<SpriteComponent>(player);

  float len = std::sqrt(ix * ix + iy * iy);
  float nx = ix / len;
  float ny = iy / len;

  // Client-side prediction step
  Vec2f predPos = trans.position + Vec2f{nx, ny} * move.speed * deltaTime;
  if (world->DoesTileBlockMovement(predPos)) {
    trans.position = predPos;
  }
  if (ix > 0) {
    playerSpriteComp.flip = SDL_FLIP_NONE;
  } else if (ix < 0) {
    playerSpriteComp.flip = SDL_FLIP_HORIZONTAL;
  }

  uint8_t inputbit{0};
  if (ix > 0)
    inputbit |= static_cast<uint8_t>(EPlayerInput::RIGHT);
  else if (ix < 0)
    inputbit |= static_cast<uint8_t>(EPlayerInput::LEFT);
  if (iy > 0)
    inputbit |= static_cast<uint8_t>(EPlayerInput::UP);
  else if (iy < 0)
    inputbit |= static_cast<uint8_t>(EPlayerInput::DOWN);
  auto& pred = registry->GetComponent<NetPredictionComponent>(player);
  uint16_t seq = pred.lastSentSeq++;

  const std::size_t bodySize =
      sClientID + sizeof(uint16_t) + sizeof(uint8_t) + sizeof(float);
  const std::size_t packetSize = sPacketHeader + bodySize;

  PacketPtr movereq = std::make_unique<uint8_t[]>(packetSize);
  uint8_t* p = movereq.get();
  util::WriteHeader(p, CLIENT_MOVE_REQ, packetSize);
  util::Write64BigEnd(p, myClientID);
  util::Write16BigEnd(p, seq);
  *p++ = inputbit;
  util::WriteF32BigEnd(p, static_cast<float>(deltaTime));
  sendQueue->Push(std::move(movereq));

  // Track pending input for future reconciliation
  pred.pending[pred.pendingCnt % 32] = {seq, nx, ny, deltaTime};
  pred.pendingCnt = static_cast<uint8_t>((pred.pendingCnt + 1) % 255);
}

void ClientNetworkSystem::SendMessage(std::shared_ptr<std::string> message) {
  PacketPtr packet =
      std::make_unique<uint8_t[]>(sHeaderAndId + message->size());

  uint8_t* p = packet.get();
  util::WriteHeader(p, PACKET::CHAT_CLIENT, sHeaderAndId + message->size());

  util::Write64BigEnd(p, myClientID);

  std::memcpy(p, message->c_str(), message->size());
  sendQueue->Push(std::move(packet));
}

ClientNetworkSystem::~ClientNetworkSystem() = default;