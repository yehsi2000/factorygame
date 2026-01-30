#pragma once

#include <memory>

#include "Core/Entity.h"
#include "Core/Event.h"
#include "Core/Item.h"
#include "Core/SystemContext.h"

class AssemblingMachineComponent;
class EventHandle;

class AssemblingMachineSystem {
  Registry *registry;
  EventDispatcher *eventDispatcher;
  TimerManager *timerManager;

  std::unique_ptr<EventHandle> AddInputEventHandle;
  std::unique_ptr<EventHandle> TakeOutputEventHandle;
  std::unique_ptr<EventHandle> CraftOutputEventHandle;

 public:
  explicit AssemblingMachineSystem(const SystemContext &context);
  ~AssemblingMachineSystem();
  void Update();

  // Inventory management
  int AddInputItem(Entity entity, ItemID itemId, int amount);
  int TakeOutputItem(Entity entity, ItemID itemId, int requestedAmount);

 private:
  void AddInputHandler(const AssemblyAddInputEvent &event);
  void TakeOutputHandler(const AssemblyTakeOutputEvent &event);

  bool HasEnoughIngredients(Entity entity) const;
  bool CanStoreOutput(Entity entity) const;

  void UpdateCrafting(Entity entity, AssemblingMachineComponent &machine,
                      float deltaTime);
  void ConsumeIngredients(Entity entity, AssemblingMachineComponent &machine);
  void ProduceOutput(Entity entity);
  void StartCrafting(Entity entity, AssemblingMachineComponent &machine);
  void UpdateAnimationState(Entity entity, AssemblingMachineComponent &machine);
};
