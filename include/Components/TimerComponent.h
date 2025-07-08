#ifndef COMPONENTS_TIMERCOMPONENT_
#define COMPONENTS_TIMERCOMPONENT_

#include <functional>

struct TimerComponent {
  float remaining;
  float duration;
  bool bIsRepeating;
  std::function<void()> onExpire;
};

#endif /* COMPONENTS_TIMERCOMPONENT_ */
