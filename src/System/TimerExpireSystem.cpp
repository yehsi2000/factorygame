#include "System/TimerExpireSystem.h"

#include "Components/TimerComponent.h"
#include "Core/Event.h"
#include "Core/GEngine.h"
#include "Core/Registry.h"
#include "Core/TimerManager.h"
#include <Commands/ResourceMineCommand.h>
#include <Components/InteractionComponent.h>
#include <Components/PlayerStateComponent.h>

void TimerExpireSystem::Update() {
  Registry* registry = engine->GetRegistry();
  auto view = registry->view<TimerExpiredTag>();
  for (auto entity : view) {
    const auto& tag =
        registry->GetComponent<TimerExpiredTag>(entity);

    // Find the handle for the expired timer
    TimerHandle handle = INVALID_TIMER_HANDLE;
    auto& timerComp =
        registry->GetComponent<TimerComponent>(entity);
    handle = timerComp.timers[static_cast<int>(tag.expiredId)];

    if (handle != INVALID_TIMER_HANDLE) {
      TimerInstance* timer = engine->GetTimerManager()->GetTimer(handle);
      if (timer) {
        switch (tag.expiredId) {
          case TimerId::Mine:
            if(registry->HasComponent<PlayerStateComponent>(entity)){
              const auto& stat = registry->GetComponent<PlayerStateComponent>(entity);
              engine->GetCommandQueue()->Enqueue(std::make_unique<ResourceMineCommand>(entity, stat.interactingEntity));
            }
            break;
          default:
            break;
        }

        if (timer->isRepeating) {
          timer->elapsed = 0.0f;
        } else {
          timerComp.timers[static_cast<int>(tag.expiredId)] =
              INVALID_TIMER_HANDLE;
          engine->GetTimerManager()->DestroyTimer(handle);
        }
      }
    }

    engine->GetRegistry()->RemoveComponent<TimerExpiredTag>(entity);
  }
}
