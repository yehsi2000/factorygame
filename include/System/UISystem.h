#ifndef SYSTEM_UISYSTEM_
#define SYSTEM_UISYSTEM_

#include "SDL.h"
#include "imgui.h"

class GEngine;

class UISystem {
 public:
  UISystem(GEngine *engine);
  void Update();
  inline void ToggleInventory() { showInventory = !showInventory; }

 private:
  GEngine *engine;
  bool showInventory = false;
  bool demoShow = true;
  void Inventory();
};

#endif /* SYSTEM_UISYSTEM_ */
