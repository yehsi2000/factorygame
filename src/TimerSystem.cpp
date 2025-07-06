#include "TimerSystem.h"

#include "Registry.h"
#include "TimerComponent.h"

void TimerSystem::Update(double dt) {
  registry->forEach<TimerComponent>([&](EntityID id, TimerComponent& timer) {
    if (timer.remaining <= 0.0f) return;
    timer.remaining -= dt;
    if (timer.remaining <= 0.0f) {
      timer.onExpire();
      if (timer.bIsRepeating) {
        timer.remaining = timer.duration;
      } else {
        registry->removeComponent<TimerComponent>(id);
      }
    }
  });
}