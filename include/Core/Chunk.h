#pragma once
 
#include <vector>

#include "Core/Entity.h"
#include "Core/TileData.h"
#include "Core/Type.h"

constexpr int CHUNK_WIDTH = 8;
constexpr int CHUNK_HEIGHT = 8;

/**
 * @brief Represents a segment of the game world.
 * @details The world is divided into chunks to manage memory and performance.
 * Each chunk contains a grid of tiles and is responsible for the entities
 * and resources within its boundaries. Chunks are loaded and unloaded
 * dynamically based on player proximity.
 */
class Chunk {
public:
  Chunk(int _chunkX, int _chunkY);
  Vec2 GetLocalTileIndex(int worldTileX, int worldTileY) const;
  TileData *GetTile(int localX, int localY);
  const TileData *GetTile(int localX, int localY) const;
  const int chunkX;
  const int chunkY;
  Entity chunkEntity = Entity::Null();

private:
  std::vector<TileData> tiles;
};
