#ifndef SYSTEM_MOVEMENTSYSTEM_
#define SYSTEM_MOVEMENTSYSTEM_

class Registry;
class TimerManager;

class MovementSystem {
  Registry* registry;
  TimerManager* timerManager;

 public:
  MovementSystem(Registry* r, TimerManager* tm);
  void Update(float deltaTime);
};

#endif /* SYSTEM_MOVEMENTSYSTEM_ */
