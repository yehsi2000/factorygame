#include "System/TimerExpireSystem.h"

#include "Components/TimerComponent.h"
#include "Event.h"
#include "GEngine.h"
#include "Registry.h"
#include "TimerManager.h"

void TimerExpireSystem::Update() {
  auto view = engine->GetRegistry()->view<TimerExpiredTag>();
  for (auto entity : view) {
    auto& tag = engine->GetRegistry()->GetComponent<TimerExpiredTag>(entity);
    
    // Find the handle for the expired timer
    TimerHandle handle = INVALID_TIMER_HANDLE;
    auto& timerComp = engine->GetRegistry()->GetComponent<TimerComponent>(entity);
    handle = timerComp.timers[static_cast<int>(tag.expiredId)];

    if (handle != INVALID_TIMER_HANDLE) {
        TimerInstance* timer = engine->GetTimerManager()->GetTimer(handle);
        if (timer) {
            // 1. Publish the high-level event
            switch (tag.expiredId) {
                case TimerId::Interact:
                    engine->GetDispatcher()->Publish(InteractEvent{});
                    break;
                default:
                    break;
            }

            // 2. Handle repeating or one-shot logic
            if (timer->isRepeating) {
                timer->elapsed = 0.0f; // Reset for the next cycle
            } else {
                // Remove handle from component and destroy the timer instance
                timerComp.timers[static_cast<int>(tag.expiredId)] = INVALID_TIMER_HANDLE;
                engine->GetTimerManager()->DestroyTimer(handle);
            }
        }
    }
    
    // 3. Remove the tag so it's not processed again
    engine->GetRegistry()->RemoveComponent<TimerExpiredTag>(entity);
  }
}
