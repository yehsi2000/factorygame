#ifndef SYSTEM_TIMERSYSTEM_
#define SYSTEM_TIMERSYSTEM_

#include "Entity.h"
#include "Registry.h"

class TimerSystem {
  Registry* registry;

 public:
  TimerSystem(Registry* r) : registry(r) {}

  void Update(float dt);
};

#endif /* SYSTEM_TIMERSYSTEM_ */
