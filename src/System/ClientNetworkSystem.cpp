#include "System/ClientNetworkSystem.h"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <format>
#include <iostream>
#include <memory>

#include "Commands/PlayerDisconnectedCommnad.h"
#include "Commands/PlayerSpawnCommand.h"
#include "Components/AnimationComponent.h"
#include "Components/InterpBufferComponent.h"
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
#include "Util/MathUtil.h"
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
      myClientID(-1),
      moveReqTimer(0.f) {
  sendChatHandle = eventDispatcher->Subscribe<SendChatEvent>(
      [this](SendChatEvent e) { SendMessage(e.message); });
}

void ClientNetworkSystem::Init(std::u8string playerName) {
  myName = std::string(reinterpret_cast<const char*>(playerName.c_str()));
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

void ClientNetworkSystem::ConnectAckHandler(const uint8_t* rp,
                                            std::size_t packetSize) {
  myClientID = util::Read64BigEnd(rp);

  uint16_t playerCnt = util::Read16BigEnd(rp);

  for (int i = 0; i < playerCnt; ++i) {
    clientid_t id = util::Read64BigEnd(rp);

    const uint8_t nameLen = *rp++;
    constexpr size_t cap = NAME_MAX_LEN - 1;

    size_t copyLen = util::utf8_clamp_to_codepoint(rp, nameLen, cap);
    std::string name(reinterpret_cast<const char*>(rp), copyLen);

    std::cout << std::format("Player {}: {}", id, name) << std::endl;

    rp += name.size();

    clientNameMap->emplace(id, name);
    commandQueue->Enqueue(std::make_unique<PlayerSpawnCommand>(id, false));
  }
  if (world->GetLocalPlayer() == INVALID_ENTITY) {
    clientNameMap->emplace(myClientID, myName);
    world->GeneratePlayer(myClientID, {0.f, 0.f}, true);
  }
}

void ClientNetworkSystem::ChatBroadcastHandler(const uint8_t* rp,
                                               std::size_t packetSize) {
  std::cout << "CHAT_BROADCAST from server" << std::endl;
  clientid_t senderClientId = util::Read64BigEnd(rp);

  if (senderClientId == myClientID) return;

  auto iter = clientNameMap->find(senderClientId);
  if (iter != clientNameMap->end()) {
    eventDispatcher->Publish(NewChatEvent(
        senderClientId,
        std::make_shared<std::string>(reinterpret_cast<const char*>(rp),
                                      packetSize - sHeaderAndId)));
  }
}

namespace {
inline double NowSeconds() {
  using clock = std::chrono::steady_clock;
  return std::chrono::duration<double>(clock::now().time_since_epoch()).count();
}
}  // namespace

// Push all snapshots (including local) into buffers. Do not write Transform
// here
void ClientNetworkSystem::TransformSnapshotHandler(const uint8_t* rp,
                                                   std::size_t /*packetSize*/) {
  const uint16_t count = util::Read16BigEnd(rp);
  const double now = NowSeconds();

  for (uint16_t i = 0; i < count; ++i) {
    clientid_t id = util::Read64BigEnd(rp);
    float posX = util::ReadF32BigEnd(rp);
    float posY = util::ReadF32BigEnd(rp);
    uint8_t facing = *rp++;

    EntityID e = world->GetPlayerByClientID(id);
    if (e == INVALID_ENTITY) continue;

    if (!registry->HasComponent<InterpBufferComponent>(e)) {
      registry->EmplaceComponent<InterpBufferComponent>(e);
    }
    auto& buf = registry->GetComponent<InterpBufferComponent>(e);

    const uint8_t writeIdx =
        static_cast<uint8_t>((buf.tail + buf.count) % InterpBufferComponent::N);
    buf.samples[writeIdx] = {now, posX, posY, facing};
    if (buf.count < InterpBufferComponent::N) {
      ++buf.count;
    } else {
      buf.tail =
          static_cast<uint8_t>((buf.tail + 1) % InterpBufferComponent::N);
    }
  }
}

static bool SampleBufferAt(const InterpBufferComponent& buf, double targetT,
                           float& outX, float& outY, uint8_t& outFacing) {
  if (buf.count == 0) return false;

  // Find s0 (<= target) and s1 (>= target)
  InterpBufferComponent::Sample s0 = buf.samples[buf.tail];
  InterpBufferComponent::Sample s1 = s0;
  bool found = false;

  for (uint8_t i = 0; i < buf.count; ++i) {
    const auto& s = buf.samples[(buf.tail + i) % InterpBufferComponent::N];
    if (s.t >= targetT) {
      s1 = s;
      if (i > 0)
        s0 = buf.samples[(buf.tail + i - 1) % InterpBufferComponent::N];
      found = true;
      break;
    }
    s0 = s;
  }

  if (!found) {
    const auto& newest =
        buf.samples[(buf.tail + buf.count - 1) % InterpBufferComponent::N];
    outX = newest.x;
    outY = newest.y;
    outFacing = newest.facing;
    return true;
  }

  if (s1.t <= s0.t + 1e-6) {
    outX = s1.x;
    outY = s1.y;
    outFacing = s1.facing;
    return true;
  }

  const double a = (targetT - s0.t) / (s1.t - s0.t);
  outX = static_cast<float>(s0.x + (s1.x - s0.x) * a);
  outY = static_cast<float>(s0.y + (s1.y - s0.y) * a);
  outFacing = s1.facing;
  return true;
}

static void ApplyPrediction(NetPredictionComponent& pred,
                            const MovementComponent& move, World* world,
                            uint8_t inputBit, float deltaTime) {
  int ix = 0, iy = 0;
  if (inputBit & static_cast<uint8_t>(EPlayerInput::RIGHT)) ix++;
  if (inputBit & static_cast<uint8_t>(EPlayerInput::LEFT)) ix--;
  if (inputBit & static_cast<uint8_t>(EPlayerInput::UP)) iy++;
  if (inputBit & static_cast<uint8_t>(EPlayerInput::DOWN)) iy--;

  if (ix != 0 || iy != 0) {
    float len = std::sqrt(static_cast<float>(ix * ix + iy * iy));
    float nx = ix / len, ny = iy / len;
    float stepX = nx * move.speed * deltaTime;
    float stepY = ny * move.speed * deltaTime;

    // lightweight client collision to reduce obvious tunneling
    Vec2f tryPos{pred.predictedX + stepX, pred.predictedY + stepY};
    if (world->IsTilePassable(tryPos)) {
      pred.predictedX = tryPos.x;
      pred.predictedY = tryPos.y;
    } else {
      // axis-separated fallback
      Vec2f tryX{pred.predictedX + stepX, pred.predictedY};
      if (world->IsTilePassable(tryX)) pred.predictedX = tryX.x;
      Vec2f tryY{pred.predictedX, pred.predictedY + stepY};
      if (world->IsTilePassable(tryY)) pred.predictedY = tryY.y;
    }
  }
}

void ClientNetworkSystem::ClientMoveResHandler(const uint8_t* rp,
                                               std::size_t packetSize) {
  uint16_t lastAckedSeq = util::Read16BigEnd(rp);
  float serverX = util::ReadF32BigEnd(rp);
  float serverY = util::ReadF32BigEnd(rp);

  EntityID me = world->GetLocalPlayer();
  if (me == INVALID_ENTITY) return;

  auto& pred = registry->GetComponent<NetPredictionComponent>(me);
  const auto& move = registry->GetComponent<MovementComponent>(me);

  // Remove acknowledged inputs from the pending list.
  auto it = pendingInputQueue.begin();
  while (it != pendingInputQueue.end()) {
    if (util::seq_leq(it->sequence, lastAckedSeq)) {
      // This input is acknowledged. Check for misprediction.
      if (it->sequence == lastAckedSeq) {
        float error = std::hypot(it->predX - serverX, it->predY - serverY);
        if (error > 0.1f) {  // Misprediction detected!
          // Rewind: Set the base of our prediction to the server's state.
          pred.predictedX = serverX;
          pred.predictedY = serverY;

          // Replay: Re-apply all inputs that came after the acknowledged one.
          for (auto replay_it = std::next(it);
               replay_it != pendingInputQueue.end(); ++replay_it) {
            ApplyPrediction(pred, move, world, replay_it->inputBit,
                            replay_it->deltaTime);
            // Update the history with the new re-predicted position.
            replay_it->predX = pred.predictedX;
            replay_it->predY = pred.predictedY;
          }
        }
      }
      it = pendingInputQueue.erase(it);
    } else {
      ++it;
    }
  }
}

// Local prediction writes to NetPredictionComponent.predicted*, not Transform
void ClientNetworkSystem::SendMoveRequest(float deltaTime) {
  EntityID localPlayer = world->GetLocalPlayer();
  if (localPlayer == INVALID_ENTITY) return;

  int ix = inputManager->GetXAxis();
  int iy = inputManager->GetYAxis();

  uint8_t inputBit{0};
  if (ix > 0)
    inputBit |= static_cast<uint8_t>(EPlayerInput::RIGHT);
  else if (ix < 0)
    inputBit |= static_cast<uint8_t>(EPlayerInput::LEFT);
  if (iy > 0)
    inputBit |= static_cast<uint8_t>(EPlayerInput::UP);
  else if (iy < 0)
    inputBit |= static_cast<uint8_t>(EPlayerInput::DOWN);

  if (registry->HasComponent<AnimationComponent>(localPlayer)) {
    auto& anim = registry->GetComponent<AnimationComponent>(localPlayer);
    auto& psc = registry->GetComponent<PlayerStateComponent>(localPlayer);
    if (ix == 0 && iy == 0) {
      if (!psc.bIsMining)
        util::SetAnimation(AnimationName::PLAYER_IDLE, anim, true);
    } else {
      util::SetAnimation(AnimationName::PLAYER_WALK, anim, true);
    }
  }
  if (registry->HasComponent<SpriteComponent>(localPlayer)) {
    auto& spr = registry->GetComponent<SpriteComponent>(localPlayer);
    if (ix > 0)
      spr.flip = SDL_FLIP_NONE;
    else if (ix < 0)
      spr.flip = SDL_FLIP_HORIZONTAL;
  }

  // Predict locally (write into NetPredictionComponent)
  auto& pred = registry->GetComponent<NetPredictionComponent>(localPlayer);
  const auto& move = registry->GetComponent<MovementComponent>(localPlayer);

  if (!pred.initialized) {
    // Initialize predicted to current transform on first use
    const auto& tr = registry->GetComponent<TransformComponent>(localPlayer);
    pred.predictedX = tr.position.x;
    pred.predictedY = tr.position.y;
    pred.offsetX = 0.f;
    pred.offsetY = 0.f;
    pred.initialized = 1;
  }

  // Apply prediction for this tick
  ApplyPrediction(pred, move, world, inputBit, deltaTime);

  // Every tick, generate a sequence number and an input command.
  inputSequenceNumber++;

  // Store input and the result for reconciliation
  pendingInputQueue.push_back({inputSequenceNumber, inputBit, pred.predictedX,
                               pred.predictedY, deltaTime});

  // Send the input to the server
  const std::size_t payloadSize = sizeof(uint16_t) + sizeof(uint8_t);
  const std::size_t packetSize = sPacketHeader + payloadSize;

  PacketPtr pkt = std::make_unique<uint8_t[]>(packetSize);
  uint8_t* p = pkt.get();
  util::WriteHeader(p, PACKET::CLIENT_MOVE_REQ, packetSize);
  util::Write16BigEnd(p, inputSequenceNumber);
  *p++ = inputBit;
  sendQueue->Push(std::move(pkt));
}

// Smoothly interpolates the visual transform to the corrected predicted
// position.
void ClientNetworkSystem::ApplyLocalSmoothing(float deltaTime) {
  EntityID localPlayer = world->GetLocalPlayer();
  if (localPlayer == INVALID_ENTITY) return;
  if (!registry->HasComponent<NetPredictionComponent>(localPlayer)) return;

  auto& pred = registry->GetComponent<NetPredictionComponent>(localPlayer);
  auto& trans = registry->GetComponent<TransformComponent>(localPlayer);

  // When a reconciliation happens, pred.predictedX/Y will jump, and this
  // will cause the visual transform to smoothly catch up over a few frames.
  const float kCatchUpSpeed = 20.0f;  // Adjustable constant for smoothing
  trans.position.x =
      util::Lerp(trans.position.x, pred.predictedX, kCatchUpSpeed * deltaTime);
  trans.position.y =
      util::Lerp(trans.position.y, pred.predictedY, kCatchUpSpeed * deltaTime);
}

// Remote interpolation for non-local players
void ClientNetworkSystem::ApplyRemoteInterpolation() {
  const double now = NowSeconds();
  constexpr double kDelay = 0.10;  // 100 ms, also adjustable constant

  for (EntityID e :
       registry->view<InterpBufferComponent, TransformComponent>()) {
    // Skip local here; handled by ApplyLocalSmoothing
    if (registry->HasComponent<LocalPlayerComponent>(e)) continue;

    auto& buf = registry->GetComponent<InterpBufferComponent>(e);
    auto& trans = registry->GetComponent<TransformComponent>(e);
    float x, y;
    uint8_t f;
    if (!SampleBufferAt(buf, now - kDelay, x, y, f)) continue;
    trans.position.x = x;
    trans.position.y = y;

    if (registry->HasComponent<SpriteComponent>(e)) {
      auto& spr = registry->GetComponent<SpriteComponent>(e);
      spr.flip = (f == 1) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    }
  }
}

void ClientNetworkSystem::Update(float deltaTime) {
  // 1) Drain incoming first
  PacketPtr packet;
  while (recvQueue->TryPop(packet)) {
    const uint8_t* rp = packet.get();
    std::size_t packetSize;
    PACKET packetId;
    util::GetHeader(rp, packetId, packetSize);
    switch (packetId) {
      case CONNECT_ACK:
        ConnectAckHandler(rp, packetSize);
        break;
      case CHAT_BROADCAST:
        ChatBroadcastHandler(rp, packetSize);
        break;
      case TRANSFORM_SNAPSHOT:
        TransformSnapshotHandler(rp, packetSize);
        break;
      case CLIENT_MOVE_RES:
        ClientMoveResHandler(rp, packetSize);
        break;
      case PLAYER_DISCONNECTED_BROADCAST: {
        clientid_t disconnectedId = util::Read64BigEnd(rp);
        std::cout << "PLAYER_DISCONNECTED_BROADCAST from server, id: "
                  << disconnectedId << std::endl;

        auto iter = clientNameMap->find(disconnectedId);
        if (iter != clientNameMap->end()) {
          clientNameMap->erase(iter);
          commandQueue->Enqueue(
              std::make_unique<PlayerDisconnectedCommand>(disconnectedId));
        }
        break;
      }
      default:
        break;
    }
  }

  // 2) Apply smoothing
  ApplyRemoteInterpolation();
  ApplyLocalSmoothing(deltaTime);

  // 3) Fixed-rate send + local prediction
  moveReqTimer += deltaTime;
  while (moveReqTimer >= syncDelta) {
    SendMoveRequest(syncDelta);
    moveReqTimer -= syncDelta;
  }

  // 4) Flush outgoing
  while (sendQueue->TryPop(packet)) {
    const uint8_t* rp = packet.get();
    std::size_t packetSize;
    PACKET packetId;
    util::GetHeader(rp, packetId, packetSize);
    connectionSocket->Send(packet.get(), packetSize);
  }
}

void ClientNetworkSystem::SendMessage(std::shared_ptr<std::string> message) {
  PacketPtr packet =
      std::make_unique<uint8_t[]>(sPacketHeader + message->size());

  uint8_t* p = packet.get();
  util::WriteHeader(p, PACKET::CHAT_CLIENT, sPacketHeader + message->size());

  std::memcpy(p, message->c_str(), message->size());
  sendQueue->Push(std::move(packet));
}

ClientNetworkSystem::~ClientNetworkSystem() = default;