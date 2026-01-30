#pragma once

#include "Core/SystemContext.h"

class AnimationSystem {
  Registry* registry;

 public:
  explicit AnimationSystem(const SystemContext& context);
  ~AnimationSystem();
  void Update(float deltaTime);
};
