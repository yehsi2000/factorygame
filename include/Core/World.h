#ifndef CORE_WORLD_
#define CORE_WORLD_

#include <map>
#include <random>

#include "Components/ChunkComponent.h"
#include "Components/TransformComponent.h"
#include "Core/Chunk.h"
#include "Core/Registry.h"
#include "Core/TileData.h"
#include "Core/Type.h"
#include "Core/World.h"
#include "SDL.h"
#include "SDL_ttf.h"

class Registry;

class GEngine;

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
  World(SDL_Renderer* renderer, Registry* registry, TTF_Font* font);

  void Update(EntityID player);
  TileData* GetTileAtWorldPosition(Vec2f position);
  TileData* GetTileAtWorldPosition(float worldX, float worldY);
  Vec2 GetTileCoordFromWorldPosition(Vec2f position);
  Vec2 GetTileCoordFromWorldPosition(float worldX, float worldY);
  TileData* GetTileAtTileCoords(Vec2 tileCoord);
  TileData* GetTileAtTileCoords(int tileX, int tileY);

  const std::map<ChunkCoord, Chunk>& GetActiveChunks() const;

 private:
  void LoadChunk(int chunkX, int chunkY);
  void GenerateChunk(Chunk& chunk);
  void UnloadChunk(Chunk& chunk);
  SDL_Texture* CreateChunkTexture(Chunk& chunk);

  TTF_Font* font;
  SDL_Renderer* renderer;
  Registry* registry;

  std::mt19937 randomGenerator;
  std::normal_distribution<float> distribution;
  std::map<ChunkCoord, Chunk> activeChunks;
  std::map<ChunkCoord, Chunk> chunkCache;
  int viewDistance = 2;  // 플레이어로부터 몇 청크까지 로드할 것인가
};

#endif /* CORE_WORLD_ */
