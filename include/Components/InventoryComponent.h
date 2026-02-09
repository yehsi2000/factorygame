#pragma once

#include <vector>

#include "Core/Item.h"

// TODO : make trivially copyable, destructable

struct InventoryComponent {
  int row = 1;
  int column = 1;
  std::vector<std::pair<ItemID, int>> items;
};
