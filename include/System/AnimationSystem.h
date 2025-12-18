#pragma once
 
#include "Core/SystemContext.h"
#include "DataStruct/AnimationData.h"

class AnimationSystem {
  Registry* registry;

 public:
  AnimationSystem(const SystemContext& context);
  ~AnimationSystem();
  void Update(float deltaTime);
};
