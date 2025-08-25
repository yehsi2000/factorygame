#ifndef SYSTEM_TIMERSYSTEM_
#define SYSTEM_TIMERSYSTEM_

class Registry;
class TimerManager;

// The TimerSystem is responsible for updating the elapsed time of all active timers.
class TimerSystem {
 public:
  TimerSystem(Registry* registry, TimerManager* timerManager);
  void Update(float deltaTime);

 private:
  Registry* registry;
  TimerManager* timerManager;
};

#endif /* SYSTEM_TIMERSYSTEM_ */
