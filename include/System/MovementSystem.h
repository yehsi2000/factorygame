#ifndef SYSTEM_MOVEMENTSYSTEM_
#define SYSTEM_MOVEMENTSYSTEM_

class Registry;
class TimerManager;
class World;

class MovementSystem {
  Registry* registry;
  TimerManager* timerManager;
  World* world;

 public:
  MovementSystem(Registry* r, TimerManager* tm, World* world);
  void Update(float deltaTime);
};

#endif /* SYSTEM_MOVEMENTSYSTEM_ */
