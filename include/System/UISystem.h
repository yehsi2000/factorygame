#ifndef SYSTEM_UISYSTEM_
#define SYSTEM_UISYSTEM_

#include <memory>

#include "Core/Entity.h"
#include "Core/SystemContext.h"
#include "Core/Item.h"

class EventHandle;

class UISystem {
  AssetManager* assetManager;
  EventDispatcher* eventDispatcher;
  Registry* registry;
  TimerManager* timerManager;
  World* world;

 public:
  UISystem(const SystemContext& context);
  ~UISystem();
  void Update();
  inline void ToggleInventory() { showInventory = !showInventory; }

 private:
  void Inventory();
  void AssemblingMachineUI();
  void AssemblingMachineRecipeSelection(EntityID entity);
  void MiningDrillUI();

  std::unique_ptr<EventHandle> showInventoryEvent;
  ItemPayload payload;
  bool showInventory = false;
  bool demoShow = false;
};

#endif /* SYSTEM_UISYSTEM_ */
