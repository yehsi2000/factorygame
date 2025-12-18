#pragma once
 
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
  void OnPlayerEndInteractEvent(const PlayerEndInteractEvent& event);
  void ResourceNodeInteractionHandler(Entity player, Entity targetEntity);
  void AssemblyMachineInteractionHandler(Entity player,
                                         Entity targetEntity);
  void MiningDrillInteractionHandler(Entity player, Entity targetEntity);

  Registry* registry;
  World* world;
  InputManager* inputManager;
  EventDispatcher* eventDispatcher;
  TimerManager* timerManager;
  std::unique_ptr<EventHandle> startInteractHandle;
  std::unique_ptr<EventHandle> endInteractHandle;
  // TODO set it as player upgradable value
  double maxInteractionDistance = 200.0;
};
