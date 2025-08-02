#include "Core/Chunk.h"

#include "Core/Type.h"

Chunk::Chunk(int _chunkX, int _chunkY) : chunkX(_chunkX), chunkY(_chunkY) {
  tiles.resize(CHUNK_WIDTH * CHUNK_HEIGHT);
}

Vec2 Chunk::GetLocalTileCoords(int worldTileX, int worldTileY) const {
  int localX = worldTileX - (chunkX * CHUNK_WIDTH);
  int localY = worldTileY - (chunkY * CHUNK_HEIGHT);

  // negative value correction
  if (localX < 0) localX += CHUNK_WIDTH;
  if (localY < 0) localY += CHUNK_HEIGHT;

  return Vec2{localX, localY};
}

TileData* Chunk::GetTile(int localX, int localY) {
  if (localX >= 0 && localX < CHUNK_WIDTH && localY >= 0 &&
      localY < CHUNK_HEIGHT) {
    return &tiles[localY * CHUNK_WIDTH + localX];
  }
  return nullptr;
}
