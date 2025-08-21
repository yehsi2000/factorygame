#ifndef SYSTEM_UISYSTEM_
#define SYSTEM_UISYSTEM_

#include "Core/EventDispatcher.h"
#include "Core/Entity.h"
#include "Core/Recipe.h"

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
  bool demoShow = true;
  ItemPayload payload;
  void Inventory();
  void AssemblingMachineUI();
  void AssemblingMachineRecipeSelection(EntityID entity);
};

#endif /* SYSTEM_UISYSTEM_ */
