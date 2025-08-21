#ifndef CORE_CHUNK_
#define CORE_CHUNK_

#include <vector>

#include "Core/Entity.h"
#include "Core/TileData.h"
#include "Core/Type.h"

constexpr int CHUNK_WIDTH = 8;
constexpr int CHUNK_HEIGHT = 8;

class Chunk {
public:
  Chunk(int _chunkX, int _chunkY);
  Vec2 GetLocalTileIndex(int worldTileX, int worldTileY) const;
  TileData *GetTile(int localX, int localY);
  const int chunkX;
  const int chunkY;
  EntityID chunkEntity = 0;

private:
  std::vector<TileData> tiles;
};

#endif /* CORE_CHUNK_ */