#ifndef SYSTEM_INTERACTIONSYSTEM_
#define SYSTEM_INTERACTIONSYSTEM_

#include <memory>

#include "Core/Event.h"
#include "Core/SystemContext.h"

class EventHandle;

class InteractionSystem {
 public:
  InteractionSystem(const SystemContext& context);
  ~InteractionSystem();
  void Update();  // In case the system needs a per-frame update in the future.

 private:
  void OnPlayerInteractEvent(const PlayerInteractEvent& event);
  void ResourceNodeInteractionHandler(EntityID player, EntityID targetEntity);
  void AssemblyMachineInteractionHandler(EntityID player,
                                         EntityID targetEntity);
  void MiningDrillInteractionHandler(EntityID player, EntityID targetEntity);

  Registry* registry;
  World* world;
  EventDispatcher* eventDispatcher;
  TimerManager* timerManager;
  std::unique_ptr<EventHandle> handle;
};

#endif/* SYSTEM_INTERACTIONSYSTEM_ */
