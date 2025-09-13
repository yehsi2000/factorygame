#include "System/MovementSystem.h"

#include <cmath>

#include "Components/AnimationComponent.h"
#include "Components/InactiveComponent.h"
#include "Components/LocalPlayerComponent.h"
#include "Components/MovableComponent.h"
#include "Components/MoveIntentComponent.h"
#include "Components/MovementComponent.h"
#include "Components/NetPredictionComponent.h"
#include "Components/PlayerStateComponent.h"
#include "Components/SpriteComponent.h"
#include "Components/TransformComponent.h"
#include "Core/Entity.h"
#include "Core/InputManager.h"
#include "Core/Registry.h"
#include "Core/TimerManager.h"
#include "Core/World.h"
#include "SDL_render.h"
#include "Util/AnimUtil.h"
#include "Util/TimerUtil.h"

MovementSystem::MovementSystem(const SystemContext &context)
    : registry(context.registry),
      inputManager(context.inputManager),
      timerManager(context.timerManager),
      eventDispatcher(context.eventDispatcher),
      world(context.world),
      bIsServer(context.bIsServer) {}

void MovementSystem::AddServerMoveIntent(float deltaTime) {
  EntityID localPlayer = world->GetLocalPlayer();
  if (localPlayer == INVALID_ENTITY) return;
  if (registry->HasComponent<MoveIntentComponent>(localPlayer)) return;
  auto &intent = registry->GetComponent<MoveIntentComponent>(localPlayer);

  int ix = inputManager->GetXAxis();
  int iy = inputManager->GetYAxis();
  uint8_t inputbit{0};
  if (ix > 0)
    inputbit |= static_cast<uint8_t>(EPlayerInput::RIGHT);
  else if (ix < 0)
    inputbit |= static_cast<uint8_t>(EPlayerInput::LEFT);
  if (iy > 0)
    inputbit |= static_cast<uint8_t>(EPlayerInput::UP);
  else if (iy < 0)
    inputbit |= static_cast<uint8_t>(EPlayerInput::DOWN);

  intent.inputBit = inputbit;
  intent.hasNew = true;
  intent.deltaTime =
      std::min(deltaTime, 1.f / 30.f);  // clamp to avoid large dt
  intent.seq++;
}

void MovementSystem::Update(float deltaTime) {
  if (bIsServer) {
    ServerUpdate(deltaTime);
  } else {
    ClientUpdate(deltaTime);
  }
}

void MovementSystem::ServerUpdate(float deltaTime) {
  for (EntityID entity : registry->view < MovableComponent, MovementComponent,
       TransformComponent) {
    if (registry->HasComponent<InactiveComponent>(entity)) {
      continue;
    }
    const MovementComponent &move =
        registry->GetComponent<MovementComponent>(entity);

    TransformComponent &trans =
        registry->GetComponent<TransformComponent>(entity);

    // Handle Player Movement
    if (registry->HasComponent<PlayerStateComponent>(entity) &&
        registry->HasComponent<MoveIntentComponent>(entity)) {
      auto &state = registry->GetComponent<PlayerStateComponent>(entity);
      auto &intent = registry->GetComponent<MoveIntentComponent>(entity);
      auto &anim = registry->GetComponent<AnimationComponent>(entity);
      auto &sprite = registry->GetComponent<SpriteComponent>(entity);

      int ix = inputManager->GetXAxis();
      int iy = inputManager->GetYAxis();

      if (ix == 0.f && iy == 0.f) {
        if (!state.bIsMining)
          util::SetAnimation(AnimationName::PLAYER_IDLE, anim, true);
        return;
      }

      // Normalize direction for uniform movement speed
      float length = std::sqrt(ix * ix + iy * iy);

      Vec2f nextPos = trans.position +
                      Vec2f{ix / length, iy / length} * move.speed * deltaTime;

      // Block movement
      if (world->DoesTileBlockMovement(nextPos)) {
        trans.position = nextPos;
      }

      util::SetAnimation(AnimationName::PLAYER_WALK, anim, true);

      // Play running animation
      if (ix > 0) {
        sprite.flip = SDL_FLIP_NONE;
      } else if (ix < 0) {
        sprite.flip = SDL_FLIP_HORIZONTAL;
      }

      // Stop player interaction
      if (state.bIsMining) {
        state.bIsMining = false;
        state.interactingEntity = INVALID_ENTITY;
        util::DetachTimer(registry, timerManager, entity, TimerId::Mine);
      }
    }
  }
}

void MovementSystem::ClientUpdate(float deltaTime) {
  EntityID player = world->GetLocalPlayer();

  auto &playerStateComp = registry->GetComponent<PlayerStateComponent>(player);

  const MovementComponent &move =
      registry->GetComponent<MovementComponent>(player);
  TransformComponent &trans =
      registry->GetComponent<TransformComponent>(player);
  auto &playerAnimComp = registry->GetComponent<AnimationComponent>(player);
  auto &playerSpriteComp = registry->GetComponent<SpriteComponent>(player);
  auto &netPredComp = registry->GetComponent<NetPredictionComponent>(player);

  int ix = inputManager->GetXAxis();
  int iy = inputManager->GetYAxis();

  if (ix == 0.f && iy == 0.f) {
    if (!playerStateComp.bIsMining)
      util::SetAnimation(AnimationName::PLAYER_IDLE, playerAnimComp, true);
    return;
  }

  // Normalize direction for uniform movement speed
  float length = std::sqrt(ix * ix + iy * iy);

  Vec2f nextPos =
      trans.position + Vec2f{ix / length, iy / length} * move.speed * deltaTime;

  // Block movement
  if (world->DoesTileBlockMovement(nextPos)) {
    trans.position = nextPos;
  }

  util::SetAnimation(AnimationName::PLAYER_WALK, playerAnimComp, true);

  // Play running animation
  if (ix > 0) {
    playerSpriteComp.flip = SDL_FLIP_NONE;
  } else if (ix < 0) {
    playerSpriteComp.flip = SDL_FLIP_HORIZONTAL;
  }

  // Stop player interaction
  if (playerStateComp.bIsMining) {
    playerStateComp.bIsMining = false;
    playerStateComp.interactingEntity = INVALID_ENTITY;
    util::DetachTimer(registry, timerManager, player, TimerId::Mine);
  }
}

MovementSystem::~MovementSystem() = default;