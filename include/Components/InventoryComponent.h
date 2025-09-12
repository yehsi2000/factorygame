#ifndef COMPONENTS_INVENTORYCOMPONENT_
#define COMPONENTS_INVENTORYCOMPONENT_

#include <vector>

#include "Core/Item.h"

struct InventoryComponent {
  int row = 1;
  int column = 1;
  std::vector<std::pair<ItemID, int>> items;
};

#endif /* COMPONENTS_INVENTORYCOMPONENT_ */
