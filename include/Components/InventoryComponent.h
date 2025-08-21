#ifndef COMPONENTS_INVENTORYCOMPONENT_
#define COMPONENTS_INVENTORYCOMPONENT_

#include <unordered_map>
#include <vector>

#include "Core/Item.h"

struct InventoryComponent {
  std::vector<std::pair<ItemID, int>> items;
  int row = 4;
  int column = 4;
};

#endif /* COMPONENTS_INVENTORYCOMPONENT_ */
