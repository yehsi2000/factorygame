#pragma once
 
#include "Core/SystemContext.h"
#include "Core/Entity.h"

class MiningDrillComponent;

class MiningDrillSystem {
 public:
  MiningDrillSystem(const SystemContext& context);
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
