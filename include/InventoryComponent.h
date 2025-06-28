#pragma once
#include <unordered_map>

#include "Item.h"

struct InventoryComponent {
  std::unordered_map<ItemID, int> items;
};
