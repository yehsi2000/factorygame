#include "System/MovementSystem.h"

#include <Util/TimerUtil.h>

#include <cmath>  // for std::sqrt

#include "Components/InactiveComponent.h"
#include "Components/MovableComponent.h"
#include "Components/MovementComponent.h"
#include "Components/TransformComponent.h"
#include "Core/Entity.h"
#include "Core/InputState.h"
#include "Core/Registry.h"
#include "Core/TimerManager.h"

MovementSystem::MovementSystem(Registry* r, TimerManager* tm)
    : registry(r), timerManager(tm) {}

void MovementSystem::Update(float deltaTime) {
  float ix = registry->GetInputState().xAxis;
  float iy = registry->GetInputState().yAxis;
  if (ix == 0.f && iy == 0.f) return;

  for (EntityID entity :
       registry
           ->view<MovableComponent, MovementComponent, TransformComponent>()) {
    if (registry->HasComponent<InactiveComponent>(entity)) {
      continue;
    }
    const MovementComponent& move =
        registry->GetComponent<MovementComponent>(entity);
    TransformComponent& trans =
        registry->GetComponent<TransformComponent>(entity);

    // Normalize direction for uniform movement speed
    float length = std::sqrt(ix * ix + iy * iy);

    trans.position.x += (ix / length) * move.speed * deltaTime;
    trans.position.y += (iy / length) * move.speed * deltaTime;
    //TODO : stop interacting
  }
}