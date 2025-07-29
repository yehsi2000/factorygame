#ifndef TILEDATA_
#define TILEDATA_

#include "Entity.h"

enum class TileType { tDirt ,tGrass, tWater, tStone };

constexpr int TILE_PIXEL_WIDTH = 64;
constexpr int TILE_PIXEL_HEIGHT = 64;


struct TileData {
  TileType type = TileType::tDirt;
  EntityID occupyingEntity = 0; // 이 타일을 차지하는 건물이나 자원의 엔티티 ID
  float debugValue;
};


#endif /* TILEDATA_ */
