#ifndef COMPONENTS_BUILDINGCOMPONENT_
#define COMPONENTS_BUILDINGCOMPONENT_

#include "Core/Type.h"
#include <vector>

struct BuildingComponent {
  int width = 1;
  int height = 1;
  // For multi-tile buildings, store the tiles this building occupies
  // This is calculated during placement and stored for cleanup during removal
  std::vector<Vec2> occupiedTiles;
};

#endif /* COMPONENTS_BUILDINGCOMPONENT_ */