#ifndef SYSTEM_MININGDRILLSYSTEM_
#define SYSTEM_MININGDRILLSYSTEM_

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
  
  void UpdateAnimationState(MiningDrillComponent& drill, EntityID entity);
  bool TileEmpty(EntityID entity);
  void StartMining(MiningDrillComponent& drill, EntityID entity);
};

#endif/* SYSTEM_MININGDRILLSYSTEM_ */
