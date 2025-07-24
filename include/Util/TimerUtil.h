#ifndef UTIL_TIMERUTIL_
#define UTIL_TIMERUTIL_

#include "Components/TimerComponent.h"

namespace util {

void AddTimer(TimerComponent& comp, TimerId id, float durSeconds, bool repeat);

void RemoveTimer(TimerComponent &comp, TimerId id);

}  // namespace util

#endif /* UTIL_TIMERUTIL_ */
