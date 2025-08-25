#ifndef SYSTEM_ASSEMBLINGMACHINESYSTEM_
#define SYSTEM_ASSEMBLINGMACHINESYSTEM_

#include "Components/AssemblingMachineComponent.h"
#include "Core/Entity.h"
#include "Core/EventDispatcher.h"
#include "Core/Item.h"

class Registry;
class TimerManager;

class AssemblingMachineSystem {
  Registry *registry;
  EventDispatcher *dispatcher;
  TimerManager *timerManager;
  EventHandle AddInputHandle;
  EventHandle TakeOutputHandle;
  EventHandle CraftOutputHandle;

public:
  AssemblingMachineSystem(Registry *registry, EventDispatcher *dispatcher,
                          TimerManager *timerManager);
  void Update();

  // Inventory management
  int AddInputItem(EntityID entity, ItemID itemId, int amount);
  int TakeOutputItem(EntityID entity, ItemID itemId, int requestedAmount);

private:
  bool HasEnoughIngredients(EntityID entity) const;
  bool CanStoreOutput(EntityID entity) const;

  void UpdateCrafting(EntityID entity, AssemblingMachineComponent &machine,
                      float deltaTime);
  void ConsumeIngredients(EntityID entity, AssemblingMachineComponent &machine);
  void ProduceOutput(EntityID entity);
  void StartCrafting(EntityID entity, AssemblingMachineComponent &machine);
  void UpdateAnimationState(EntityID entity,
                            AssemblingMachineComponent &machine);
};

#endif /* SYSTEM_ASSEMBLINGMACHINESYSTEM_ */
