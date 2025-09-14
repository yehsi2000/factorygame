#ifndef CORE_TILEDATA_
#define CORE_TILEDATA_

#include "Core/Entity.h"

/**
 * @brief Defines the different types of tiles that can exist in the world.
 */
enum class TileType { Invalid, Dirt, Grass, Water, Stone };

constexpr int TILE_PIXEL_SIZE = 64;

/**
 * @brief Contains the state and properties of a single tile in the game world.
 * @details This struct holds all the information for a tile, including its
 * terrain type and references to any entities that may be occupying it, such
 * as buildings or resource nodes.
 */
struct TileData {
  TileType type = TileType::Invalid;
  EntityID occupyingEntity = INVALID_ENTITY;
  EntityID oreEntity = INVALID_ENTITY;
  float debugValue;
};

#endif /* CORE_TILEDATA_ */
