#include "System/TimerSystem.h"

#include "Components/TimerComponent.h"
#include "Core/Registry.h"
#include "Core/TimerManager.h"

TimerSystem::TimerSystem(const SystemContext& context)
    : registry(context.registry), timerManager(context.timerManager) {}

void TimerSystem::Update(float deltaTime) {
  // Iterate over all entities that have a TimerComponent.
  auto view = registry->view<TimerComponent>();
  for (auto entity : view) {
    const auto& timerComp = registry->GetComponent<TimerComponent>(entity);

    // Check each poFtential timer handle in the component.
    for (TimerHandle handle : timerComp.timers) {
      if (handle == INVALID_TIMER_HANDLE) {
        continue;
      }

      TimerInstance* timer = timerManager->GetTimer(handle);
      if (!timer || !timer->bIsActive || timer->bIsPaused) {
        continue;
      }

      timer->elapsed += deltaTime;

      if (timer->elapsed >= timer->duration) {
        // Timer has expired. Add a tag for the TimerExpireSystem to process.
        // This decouples the timer update from the expiration logic.
        registry->EmplaceComponent<TimerExpiredTag>(entity,
                                                    TimerExpiredTag{timer->id});
      }
    }
  }
}

TimerSystem::~TimerSystem() = default;