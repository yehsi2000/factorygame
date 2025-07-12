#include "System/MovementSystem.h"

#include <cmath>  // for std::sqrt

#include "Components/MovableComponent.h"
#include "Components/MovementComponent.h"
#include "Components/TransformComponent.h"
#include "InputState.h"
#include "Entity.h"
#include "Registry.h"

MovementSystem::MovementSystem(Registry* r) : registry(r) {}

void MovementSystem::Update(float deltaTime) {
  float ix = registry->GetInputState().xAxis;
  float iy = registry->GetInputState().yAxis;
  if(ix == 0.f && iy == 0.f) return;
  
  for (EntityID entity :
       registry->view<MovableComponent, MovementComponent, TransformComponent>()) {
    MovementComponent& move = registry->GetComponent<MovementComponent>(entity);
    TransformComponent& trans =
        registry->GetComponent<TransformComponent>(entity);

    // 대각선 이동이 더 빨라지는 것을 막기 위해 방향 벡터를 정규화
    float length = std::sqrt(ix * ix + iy * iy);

    trans.xPos +=
        (ix / length) * move.speed * deltaTime;
    trans.yPos +=
        (iy / length) * move.speed * deltaTime;
  }
}