#include "System/MovementSystem.h"

#include <cmath>  // for std::sqrt

#include "Components/MovementComponent.h"
#include "Components/TransformComponent.h"
#include "Entity.h"
#include "Registry.h"

MovementSystem::MovementSystem(Registry* r) : registry(r) {}

void MovementSystem::Update(float deltaTime) {
  for (EntityID entity :
       registry->view<MovementComponent, TransformComponent>()) {
    MovementComponent& move = registry->getComponent<MovementComponent>(entity);
    TransformComponent& trans =
        registry->getComponent<TransformComponent>(entity);
    if (move.dx == 0.f && move.dy == 0.f) {
      return;
    }

    // 대각선 이동이 더 빨라지는 것을 막기 위해 방향 벡터를 정규화
    float length = std::sqrt(move.dx * move.dx + move.dy * move.dy);

    trans.xPos +=
        (move.dx / length) * move.speed * deltaTime;
    trans.yPos +=
        (move.dy / length) * move.speed * deltaTime;
  }
}