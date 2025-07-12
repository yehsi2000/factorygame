#include "System/TimerSystem.h"

#include "Components/TimerComponent.h"
#include "Registry.h"

void TimerSystem::Update(float dt) {
  registry->forEach<TimerComponent>([&](EntityID id, TimerComponent& timer) {
    if (timer.remaining <= 0.f) return;
    timer.remaining -= dt;
    if (timer.remaining <= 0.f) {
      timer.onExpire();
      if (timer.bIsRepeating) {
        timer.remaining = timer.duration;
      } else {
        registry->RemoveComponent<TimerComponent>(id);
      }
    }
  });
}