#ifndef SYSTEM_MOVEMENTSYSTEM_
#define SYSTEM_MOVEMENTSYSTEM_

#include "Core/Registry.h"
#include "Core/TimerManager.h"

class MovementSystem {
  Registry* registry;
  TimerManager* timerManager;

 public:
  MovementSystem(Registry* r, TimerManager* tm);
  void Update(float deltaTime);
};

#endif /* SYSTEM_MOVEMENTSYSTEM_ */
