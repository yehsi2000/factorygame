#pragma once

#include "Core/Entity.h"
#include "Core/SystemContext.h"

class MiningDrillComponent;

class MiningDrillSystem {
 public:
  explicit MiningDrillSystem(const SystemContext& context);
  ~MiningDrillSystem();
  void Update();

 private:
  Registry* registry;
  World* world;
  TimerManager* timerManager;

  void UpdateAnimationState(MiningDrillComponent& drill, Entity entity);
  bool TileEmpty(Entity entity);
  void StartMining(MiningDrillComponent& drill, Entity entity);
};
