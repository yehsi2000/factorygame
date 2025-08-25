#ifndef SYSTEM_MININGDRILLSYSTEM_
#define SYSTEM_MININGDRILLSYSTEM_

#include "Components/MiningDrillComponent.h"
#include "Core/Entity.h"
#include "Core/Registry.h"
#include "Core/TimerManager.h"
#include "Core/World.h"

class MiningDrillSystem {
 public:
  MiningDrillSystem(Registry* registry, World* world,
                    TimerManager* timerManager);
  void Update();

 private:
  Registry* registry;
  World* world;
  TimerManager* timerManager;
  
  void UpdateAnimationState(MiningDrillComponent& drill, EntityID entity);
  bool TileEmpty(EntityID entity);
  void StartMining(MiningDrillComponent& drill, EntityID entity);
};

#endif /* SYSTEM_MININGDRILLSYSTEM_ */
