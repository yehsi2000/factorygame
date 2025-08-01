#ifndef CORE_CHUNK_
#define CORE_CHUNK_

#include <vector>

#include "Core/TileData.h"
#include "Core/Type.h"

constexpr int CHUNK_WIDTH = 8;
constexpr int CHUNK_HEIGHT = 8;

class Chunk {
 public:
  Chunk(int _chunkX, int _chunkY);

  // 월드 좌표를 이 청크의 로컬 타일 좌표로 변환
  Vec2 GetLocalTileCoords(int worldTileX, int worldTileY) const;
  TileData* GetTile(int localX, int localY);

  // 청크의 좌표 (타일 좌표 기준 아님)
  const int chunkX;
  const int chunkY;

 private:
  std::vector<TileData> tiles;
};

#endif /* CORE_CHUNK_ */
