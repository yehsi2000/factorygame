#ifndef SYSTEM_ANIMATIONSYSTEM_
#define SYSTEM_ANIMATIONSYSTEM_

#include "Core/Registry.h"

class AnimationSystem {
  Registry* registry;

 public:
  AnimationSystem(Registry* r);
  void Update(float deltaTime);
};

#endif /* SYSTEM_ANIMATIONSYSTEM_ */
