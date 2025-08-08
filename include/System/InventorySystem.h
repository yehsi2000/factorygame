#ifndef SYSTEM_INVENTORYSYSTEM_
#define SYSTEM_INVENTORYSYSTEM_

#include "Components/InventoryComponent.h"
#include "Core/Event.h"
#include "Core/EventDispatcher.h"
#include "Core/Item.h"

class GEngine;

class InventorySystem {
 public:
  InventorySystem(GEngine *engine);

 private:
  EventHandle addEventHandle;
  EventHandle consumeEventHandle;
  GEngine *engine;
  void AddItem(const ItemAddEvent &e);
  void ConsumeItem(const ItemConsumeEvent &e);
};

#endif /* SYSTEM_INVENTORYSYSTEM_ */
