#ifndef SYSTEM_INVENTORYSYSTEM_
#define SYSTEM_INVENTORYSYSTEM_

#include <memory>

#include "Components/InventoryComponent.h"
#include "Core/Item.h"

class InventorySystem {
 public:
  InventorySystem() {}
  ~InventorySystem();
  bool consume(InventoryComponent& inv, ItemID itemID, int n);
  void add(InventoryComponent& inv, ItemID itemID, int n);
  int get(InventoryComponent& inv, ItemID itemID) const;
};

#endif /* SYSTEM_INVENTORYSYSTEM_ */
