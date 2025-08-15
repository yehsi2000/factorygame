#ifndef SYSTEM_UISYSTEM_
#define SYSTEM_UISYSTEM_

#include "Core/EventDispatcher.h"
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
  void Inventory();
};

#endif /* SYSTEM_UISYSTEM_ */
