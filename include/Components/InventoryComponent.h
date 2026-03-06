#pragma once

#include <vector>

#include "Core/Item.h"

struct InventoryComponent {
  int row = 1;
  int column = 1;
  std::vector<std::pair<ItemID, int>> items;
};

// TODO : make trivially copyable, destructable

#ifndef DISABLE_COMP_TYPECHECK
#include <type_traits>
// static_assert(std::is_trivially_copyable_v<InventoryComponent> == true);
// static_assert(std::is_trivially_destructible_v<InventoryComponent> == true);
#endif