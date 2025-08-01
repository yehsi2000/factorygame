#ifndef SYSTEM_INVENTORYSYSTEM_
#define SYSTEM_INVENTORYSYSTEM_

#include <memory>

#include "Components/InventoryComponent.h"
#include "Core/Item.h"

class InventorySystem {
 public:
  InventorySystem(std::shared_ptr<ItemDatabase> db) : itemDB(db) {}
  ~InventorySystem();
  bool consume(InventoryComponent& inv, ItemID itemID, int n);
  void add(InventoryComponent& inv, ItemID itemID, int n);
  int get(InventoryComponent& inv, ItemID itemID) const;

 private:
  std::shared_ptr<ItemDatabase> itemDB;
};

#endif /* SYSTEM_INVENTORYSYSTEM_ */
