#include "System/MovementSystem.h"

#include <cmath>

#include "Components/AnimationComponent.h"
#include "Components/InactiveComponent.h"
#include "Components/InputStateComponent.h"
#include "Components/LocalPlayerComponent.h"
#include "Components/MovableComponent.h"
#include "Components/MovementComponent.h"
#include "Components/PlayerStateComponent.h"
#include "Components/SpriteComponent.h"
#include "Components/TransformComponent.h"
#include "Core/Entity.h"
#include "Core/Event.h"
#include "Core/InputManager.h"
#include "Core/Packet.h"
#include "Core/Registry.h"
#include "Core/TimerManager.h"
#include "Core/World.h"
#include "SDL_render.h"
#include "Util/AnimUtil.h"
#include "Util/PacketUtil.h"
#include "Util/TimerUtil.h"


MovementSystem::MovementSystem(const SystemContext& context)
    : registry(context.registry),
      inputManager(context.inputManager),
      timerManager(context.timerManager),
      eventDispatcher(context.eventDispatcher),
      world(context.world),
      bIsServer(context.bIsServer),
      pendingMoves(context.pendingMoves) {}

void MovementSystem::Update(float deltaTime) {
  if (bIsServer) {
    ServerUpdate(deltaTime);
  } else {
    // ClientUpdate(deltaTime);
  }
}

void MovementSystem::ServerUpdate(float deltaTime) {
  // Clamp large spikes to keep physics stable but still use real deltaTime
  constexpr float kMaxStep = 1.f / 20.f; // 50 ms
  if (deltaTime > kMaxStep) deltaTime = kMaxStep;

  // Pre-pass: write host (server-local) input into InputStateComponent
  {
    EntityID localPlayer = world->GetLocalPlayer();
    if (localPlayer != INVALID_ENTITY && registry->HasComponent<PlayerStateComponent>(localPlayer)) {
      int ix = inputManager->GetXAxis();
      int iy = inputManager->GetYAxis();
      uint8_t bit{0};
      if (ix > 0)      bit |= static_cast<uint8_t>(EPlayerInput::RIGHT);
      else if (ix < 0) bit |= static_cast<uint8_t>(EPlayerInput::LEFT);
      if (iy > 0)      bit |= static_cast<uint8_t>(EPlayerInput::UP);
      else if (iy < 0) bit |= static_cast<uint8_t>(EPlayerInput::DOWN);

      if (!registry->HasComponent<InputStateComponent>(localPlayer)) {
        registry->EmplaceComponent<InputStateComponent>(localPlayer);
      }
      registry->GetComponent<InputStateComponent>(localPlayer).inputBit = bit;
    }
  }

  // Apply movement for all players using current input state
  for (EntityID e :
       registry->view<MovableComponent, MovementComponent, TransformComponent>()) {
    if (registry->HasComponent<InactiveComponent>(e)) continue;
    if (!registry->HasComponent<PlayerStateComponent>(e)) continue;
    if (!registry->HasComponent<InputStateComponent>(e)) continue;

    auto& psc        = registry->GetComponent<PlayerStateComponent>(e);
    auto& trans      = registry->GetComponent<TransformComponent>(e);
    const auto& move = registry->GetComponent<MovementComponent>(e);
    auto& in         = registry->GetComponent<InputStateComponent>(e);

    int ix = 0, iy = 0;
    if (in.inputBit & static_cast<uint8_t>(EPlayerInput::RIGHT)) ix++;
    if (in.inputBit & static_cast<uint8_t>(EPlayerInput::LEFT))  ix--;
    if (in.inputBit & static_cast<uint8_t>(EPlayerInput::UP))    iy++;
    if (in.inputBit & static_cast<uint8_t>(EPlayerInput::DOWN))  iy--;

    // Animation and facing if present
    if (registry->HasComponent<AnimationComponent>(e)) {
      auto& anim = registry->GetComponent<AnimationComponent>(e);
      if (ix == 0 && iy == 0) {
        util::SetAnimation(AnimationName::PLAYER_IDLE, anim, true);
      } else {
        util::SetAnimation(AnimationName::PLAYER_WALK, anim, true);
      }
    }
    if (registry->HasComponent<SpriteComponent>(e)) {
      auto& spr = registry->GetComponent<SpriteComponent>(e);
      if (ix > 0)      spr.flip = SDL_FLIP_NONE;
      else if (ix < 0) spr.flip = SDL_FLIP_HORIZONTAL;
    }

    if (ix == 0 && iy == 0) continue;

    float len = std::sqrt(static_cast<float>(ix * ix + iy * iy));
    Vec2f dir{ ix / len, iy / len };

    Vec2f next = trans.position + dir * move.speed * deltaTime;
    if (world->IsTilePassable(next)) {
      trans.position = next;
    }

    // After movement, if it was based on a client request, queue a response.
    if (in.sequence != 0) {
      pendingMoves->Push(
          {psc.clientID, in.sequence, trans.position.x, trans.position.y});
      in.sequence = 0;  // Consume the input sequence
    }
  }
}

void MovementSystem::ClientUpdate(float deltaTime) {
  for (EntityID entity :
       registry
           ->view<MovableComponent, MovementComponent, TransformComponent>()) {
    if (registry->HasComponent<InactiveComponent>(entity)) {
      continue;
    }
    const auto& move = registry->GetComponent<MovementComponent>(entity);

    auto& trans = registry->GetComponent<TransformComponent>(entity);

    // Don't process player, only other movable
    if (registry->HasComponent<PlayerStateComponent>(entity)) continue;

    // TODO : Simple movement for non-player entities
  }
}

MovementSystem::~MovementSystem() = default;