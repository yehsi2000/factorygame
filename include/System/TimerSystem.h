#pragma once
 
#include "Core/SystemContext.h"

/**
 * @brief System responsible for updating the elapsed time of all active timers
 *
 */
class TimerSystem {
  Registry* registry;
  TimerManager* timerManager;

 public:
  explicit TimerSystem(const SystemContext& context);
  ~TimerSystem();
  void Update(float deltaTime);
};
