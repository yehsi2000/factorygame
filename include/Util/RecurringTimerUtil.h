#ifndef UTIL_RECURRINGTIMERUTIL_H_
#define UTIL_RECURRINGTIMERUTIL_H_

#include "Components/TimerComponent.h"
#include "Entity.h"
#include "Registry.h"
#include <functional>

namespace util {

// Create a recurring timer for an entity
void CreateRecurringTimer(Registry* registry, EntityID entity, TimerId id, float duration);

// Create a recurring timer with a custom callback (using the TimerManager)
void CreateRecurringTimerWithCallback(Registry* registry, EntityID entity, TimerId id, float duration, 
                                      std::function<void()> callback);

// Remove a recurring timer from an entity
void RemoveRecurringTimer(Registry* registry, EntityID entity, TimerId id);

// Pause a recurring timer
void PauseRecurringTimer(Registry* registry, EntityID entity, TimerId id);

// Resume a paused recurring timer
void ResumeRecurringTimer(Registry* registry, EntityID entity, TimerId id);

// Check if a timer is recurring
bool IsTimerRecurring(Registry* registry, EntityID entity, TimerId id);

// Get remaining time for a timer
float GetRemainingTime(Registry* registry, EntityID entity, TimerId id);

}  // namespace util

#endif /* UTIL_RECURRINGTIMERUTIL_H_ */