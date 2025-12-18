#pragma once
 
#include "DataStruct/Type.h"
#include <vector>

// TODO : make trivially copyable, destructable

struct BuildingComponent {
  int width = 1;
  int height = 1;
  // For multi-tile buildings, store the tiles this building occupies
  // This is calculated during placement and stored for cleanup during removal
  std::vector<Vec2> occupiedTiles;
};
