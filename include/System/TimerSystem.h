#ifndef SYSTEM_TIMERSYSTEM_
#define SYSTEM_TIMERSYSTEM_

#include "Core/SystemContext.h"

/**
 * @brief System responsible for updating the elapsed time of all active timers
 *
 */
class TimerSystem {
  Registry* registry;
  TimerManager* timerManager;

 public:
  TimerSystem(const SystemContext& context);
  ~TimerSystem();
  void Update(float deltaTime);
};

#endif /* SYSTEM_TIMERSYSTEM_ */
