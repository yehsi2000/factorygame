#include "Entity.h"

class Registry;

class TimerSystem {
  Registry* registry;

 public:
  TimerSystem(Registry* r) : registry(r) {}

  void Update(double dt);
};
