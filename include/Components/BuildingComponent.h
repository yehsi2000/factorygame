#pragma once

#include <vector>

#include "DataStruct/Type.h"

struct BuildingComponent {
  int width = 1;
  int height = 1;

  // For multi-tile buildings, store the tiles this building occupies
  // This is calculated during placement and stored for cleanup during removal
  std::vector<Vec2> occupiedTiles;
};

// TODO : make trivially copyable, destructable

#ifndef DISABLE_COMP_TYPECHECK
#include <type_traits>
// static_assert(std::is_trivially_copyable_v<BuildingComponent> == true);
// static_assert(std::is_trivially_destructible_v<BuildingComponent> == true);
#endif