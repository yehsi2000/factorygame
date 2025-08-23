#ifndef SYSTEM_UISYSTEM_
#define SYSTEM_UISYSTEM_

#include "Core/EventDispatcher.h"
#include "Core/Entity.h"

class GEngine;

class UISystem {
 public:
  UISystem(GEngine *engine);
  void Update();
  inline void ToggleInventory() { showInventory = !showInventory; }

 private:
  GEngine *engine;
  EventHandle showInventoryEvent;
  bool showInventory = false;
  bool demoShow = false;
  ItemPayload payload;
  void Inventory();
  void AssemblingMachineUI();
  void AssemblingMachineRecipeSelection(EntityID entity);
  void MiningDrillUI();
};

#endif /* SYSTEM_UISYSTEM_ */
