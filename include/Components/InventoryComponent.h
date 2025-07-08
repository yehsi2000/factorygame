#ifndef COMPONENTS_INVENTORYCOMPONENT_
#define COMPONENTS_INVENTORYCOMPONENT_

#include <unordered_map>

#include "Item.h"

struct InventoryComponent {
  std::unordered_map<ItemID, int> items;
};

#endif /* COMPONENTS_INVENTORYCOMPONENT_ */
