#ifndef SYSTEM_ANIMATIONSYSTEM_
#define SYSTEM_ANIMATIONSYSTEM_

#include "Core/SystemContext.h"

class AnimationSystem {
  Registry* registry;

 public:
  AnimationSystem(const SystemContext& context);
  ~AnimationSystem();
  void Update(float deltaTime);
};

#endif /* SYSTEM_ANIMATIONSYSTEM_ */
