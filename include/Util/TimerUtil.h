#ifndef UTIL_TIMERUTIL_
#define UTIL_TIMERUTIL_

class Registry;  // Forward declaration
#include "Entity.h"
#include "Components/TimerComponent.h"

namespace util {

void AddTimer(TimerComponent& comp, TimerId id, float durSeconds, bool repeat = false);

void RemoveTimer(TimerComponent &comp, TimerId id);

void ResetTimerForRepeat(TimerInstance& timer);

bool IsTimerExpired(const TimerInstance& timer);

void CreateTimerOnEntity(Registry* registry, EntityID entity, TimerId id, float duration, bool repeat = false);

}  // namespace util

#endif /* UTIL_TIMERUTIL_ */
