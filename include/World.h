#ifndef WORLD_
#define WORLD_

#include <map>
#include <random>

#include "Chunk.h"
#include "Components/TransformComponent.h"  // Vec2 같은 타입을 위해 필요할 수 있음
#include "Components/ChunkComponent.h"
#include "Type.h"
#include "TileData.h"
#include "SDL.h"

class GEngine;

// 청크 좌표를 맵의 키로 사용하기 위한 구조체
struct ChunkCoord {
  int x, y;
  bool operator<(const ChunkCoord& other) const {
    if (y < other.y) return true;
    if (y > other.y) return false;
    return x < other.x;
  }
};

class World {
 public:
  World(GEngine* engine);

  // 플레이어 위치를 기반으로 주변 청크를 로드/생성/언로드
  void Update(Vec2f playerPosition);
  TileData* GetTileAtWorldPosition(float worldX, float worldY);
  TileData* GetTileAtTileCoords(int tileX, int tileY);

  const std::map<ChunkCoord, Chunk>& GetActiveChunks() const;

 private:
  void LoadChunk(int chunkX, int chunkY);
  void GenerateChunk(Chunk& chunk);  // 실제 절차적 생성 로직
  void UnloadChunk(Chunk& chunk);
  SDL_Texture* CreateChunkTexture(Chunk& chunk);

  GEngine* engine;
  std::mt19937 randomGenerator;
  std::normal_distribution<float> distribution;
  std::map<ChunkCoord, Chunk> activeChunks;
  int viewDistance = 2;  // 플레이어로부터 몇 청크까지 로드할 것인가
};

#endif /* WORLD_ */
