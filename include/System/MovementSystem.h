#ifndef SYSTEM_MOVEMENTSYSTEM_
#define SYSTEM_MOVEMENTSYSTEM_

#include "Core/SystemContext.h"

class MovementSystem {
  Registry* registry;
  TimerManager* timerManager;
  InputPoller* inputPoller;
  World* world;

 public:
  MovementSystem(const SystemContext& context);
  ~MovementSystem();
  void Update(float deltaTime);
};

#endif /* SYSTEM_MOVEMENTSYSTEM_ */
