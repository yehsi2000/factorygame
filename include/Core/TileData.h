#ifndef CORE_TILEDATA_
#define CORE_TILEDATA_

#include "Core/Entity.h"

enum class TileType { Invalid, Dirt, Grass, Water, Stone };

constexpr int TILE_PIXEL_WIDTH = 64;
constexpr int TILE_PIXEL_HEIGHT = 64;

struct TileData {
  TileType type = TileType::Invalid;
  EntityID occupyingEntity = INVALID_ENTITY;  // entity on this tile
  float debugValue;
};

#endif /* CORE_TILEDATA_ */
