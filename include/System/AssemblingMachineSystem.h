#ifndef SYSTEM_ASSEMBLINGMACHINESYSTEM_
#define SYSTEM_ASSEMBLINGMACHINESYSTEM_

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
  AssemblingMachineSystem(const SystemContext &context);
  ~AssemblingMachineSystem();
  void Update();

  // Inventory management
  int AddInputItem(EntityID entity, ItemID itemId, int amount);
  int TakeOutputItem(EntityID entity, ItemID itemId, int requestedAmount);

 private:
  void AddInputHandler(const AssemblyAddInputEvent &event);
  void TakeOutputHandler(const AssemblyTakeOutputEvent &event);

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
