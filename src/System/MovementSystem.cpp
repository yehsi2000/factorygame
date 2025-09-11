#include "System/MovementSystem.h"

#include <cmath>

#include "Core/Entity.h"
#include "Core/Registry.h"
#include "Core/TimerManager.h"
#include "Core/InputManager.h"
#include "Core/World.h"

#include "Components/AnimationComponent.h"
#include "Components/InactiveComponent.h"
#include "Components/MovableComponent.h"
#include "Components/MovementComponent.h"
#include "Components/PlayerStateComponent.h"
#include "Components/SpriteComponent.h"
#include "Components/TransformComponent.h"

#include "SDL_render.h"
#include "Util/AnimUtil.h"
#include "Util/TimerUtil.h"

MovementSystem::MovementSystem(const SystemContext& context)
    : registry(context.registry), inputManager(context.inputManager), timerManager(context.timerManager), world(context.world) {}

void MovementSystem::Update(float deltaTime) {
  for (EntityID entity :
       registry
           ->view<MovableComponent, MovementComponent, TransformComponent>()) {
    if (registry->HasComponent<InactiveComponent>(entity)) {
      continue;
    }
    const MovementComponent &move =
        registry->GetComponent<MovementComponent>(entity);
    TransformComponent &trans =
        registry->GetComponent<TransformComponent>(entity);

    // Handle Player Movement
    if (registry->HasComponent<PlayerStateComponent>(entity)) {
      auto &playerStateComp =
          registry->GetComponent<PlayerStateComponent>(entity);
      auto &playerAnimComp = registry->GetComponent<AnimationComponent>(entity);
      auto &playerSpriteComp = registry->GetComponent<SpriteComponent>(entity);

      float ix = inputManager->GetXAxis();
      float iy = inputManager->GetYAxis();

      if (ix == 0.f && iy == 0.f) {
        if(!playerStateComp.bIsMining) util::SetAnimation(AnimationName::PLAYER_IDLE, playerAnimComp, true);
        return;
      }

      // Normalize direction for uniform movement speed
      float length = std::sqrt(ix * ix + iy * iy);

      Vec2f nextPos = trans.position + Vec2f{ix/length, iy/length} * move.speed * deltaTime;

      // Block movement
      if(world->DoesTileBlockMovement(nextPos)){
        trans.position = nextPos;
      }

      util::SetAnimation(AnimationName::PLAYER_WALK, playerAnimComp, true);

      // Play running animation
      if (ix > 0) {
        playerSpriteComp.flip = SDL_FLIP_NONE;
      } else if(ix < 0) {
        playerSpriteComp.flip = SDL_FLIP_HORIZONTAL;
      }

      // Stop player interaction
      if (playerStateComp.bIsMining) {
        playerStateComp.bIsMining = false;
        playerStateComp.interactingEntity = INVALID_ENTITY;
        util::DetachTimer(registry, timerManager, entity, TimerId::Mine);
      }
    }
  }
}

MovementSystem::~MovementSystem() = default;