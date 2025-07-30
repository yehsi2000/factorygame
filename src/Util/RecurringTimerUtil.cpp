#include "Util/RecurringTimerUtil.h"
#include "Util/TimerUtil.h"
#include <algorithm>

namespace util {

void CreateRecurringTimer(Registry* registry, EntityID entity, TimerId id, float duration) {
    if (!registry->HasComponent<TimerComponent>(entity)) {
        registry->EmplaceComponent<TimerComponent>(entity);
    }
    
    auto& timerComp = registry->GetComponent<TimerComponent>(entity);
    AddTimer(timerComp, id, duration, true);  // true for repeating
}

void CreateRecurringTimerWithCallback(Registry* registry, EntityID entity, TimerId id, float duration, 
                                      std::function<void()> callback) {
    // This would typically involve creating an event and scheduling it with the TimerManager
    // For now, we'll just create the recurring timer
    CreateRecurringTimer(registry, entity, id, duration);
    
    // In a full implementation, you would set up the callback to be executed when the timer expires
    // This might involve creating a custom event type or using a more sophisticated event system
}

void RemoveRecurringTimer(Registry* registry, EntityID entity, TimerId id) {
    if (registry->HasComponent<TimerComponent>(entity)) {
        auto& timerComp = registry->GetComponent<TimerComponent>(entity);
        RemoveTimer(timerComp, id);
        
        // Clean up the component if no active timers remain
        if (timerComp.activeCount == 0) {
            registry->RemoveComponent<TimerComponent>(entity);
        }
    }
}

void PauseRecurringTimer(Registry* registry, EntityID entity, TimerId id) {
    if (registry->HasComponent<TimerComponent>(entity)) {
        auto& timerComp = registry->GetComponent<TimerComponent>(entity);
        size_t index = static_cast<size_t>(id);
        if (index < MAX_TIMERS_PER_ENTITY) {
            timerComp.timers[index].isPaused = true;
        }
    }
}

void ResumeRecurringTimer(Registry* registry, EntityID entity, TimerId id) {
    if (registry->HasComponent<TimerComponent>(entity)) {
        auto& timerComp = registry->GetComponent<TimerComponent>(entity);
        size_t index = static_cast<size_t>(id);
        if (index < MAX_TIMERS_PER_ENTITY) {
            timerComp.timers[index].isPaused = false;
        }
    }
}

bool IsTimerRecurring(Registry* registry, EntityID entity, TimerId id) {
    if (registry->HasComponent<TimerComponent>(entity)) {
        auto& timerComp = registry->GetComponent<TimerComponent>(entity);
        size_t index = static_cast<size_t>(id);
        if (index < MAX_TIMERS_PER_ENTITY) {
            return timerComp.timers[index].isRepeating;
        }
    }
    return false;
}

float GetRemainingTime(Registry* registry, EntityID entity, TimerId id) {
    if (registry->HasComponent<TimerComponent>(entity)) {
        auto& timerComp = registry->GetComponent<TimerComponent>(entity);
        size_t index = static_cast<size_t>(id);
        if (index < MAX_TIMERS_PER_ENTITY) {
            auto& timer = timerComp.timers[index];
            if (timer.isActive) {
                return std::max(0.0f, timer.duration - timer.elapsed);
            }
        }
    }
    return 0.0f;
}

}  // namespace util