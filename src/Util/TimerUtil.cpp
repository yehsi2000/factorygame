#include "Util/TimerUtil.h"

#include "Core/Registry.h"
#include "Core/TimerManager.h"

namespace util {

void AttachTimer(Registry* registry, TimerManager* timerManager,
                 EntityID entity, TimerId id, float duration,
                 bool isRepeating) {
  // Ensure the entity has a TimerComponent. If not, add one.
  if (!registry || !timerManager) return;
  if (!registry->HasComponent<TimerComponent>(entity)) {
    registry->EmplaceComponent<TimerComponent>(entity);
  }
  auto& timerComp = registry->GetComponent<TimerComponent>(entity);
  int timerIndex = static_cast<int>(id);

  // If a timer of this type already exists, detach it first.
  if (timerComp.timers[timerIndex] != INVALID_TIMER_HANDLE) {
    DetachTimer(registry, timerManager, entity, id);
  }

  // Create the new timer in the manager and get a handle.
  TimerHandle handle = timerManager->CreateTimer(id, duration, isRepeating);

  // Store the handle in the entity's component.
  timerComp.timers[timerIndex] = handle;
}

void DetachTimer(Registry* registry, TimerManager* timerManager,
                 EntityID entity, TimerId id) {
  if (!registry || !timerManager) return;
  if (!registry->HasComponent<TimerComponent>(entity)) {
    return;  // Nothing to detach.
  }

  auto& timerComp = registry->GetComponent<TimerComponent>(entity);
  int timerIndex = static_cast<int>(id);
  TimerHandle handle = timerComp.timers[timerIndex];

  if (handle != INVALID_TIMER_HANDLE) {
    // Destroy the timer instance in the manager.
    timerManager->DestroyTimer(handle);
    // Invalidate the handle in the component.
    timerComp.timers[timerIndex] = INVALID_TIMER_HANDLE;
  }
}

}  // namespace util
