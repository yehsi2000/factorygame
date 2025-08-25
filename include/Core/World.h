#ifndef CORE_WORLD_
#define CORE_WORLD_

#include <map>
#include <random>

#include "Components/ResourceNodeComponent.h"
#include "Core/Chunk.h"
#include "Core/Registry.h"
#include "Core/TileData.h"
#include "Core/Type.h"
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
  friend class TestWorld;

 public:
  World(SDL_Renderer* renderer, Registry* registry, TTF_Font* font);

  void Update(EntityID player);
  TileData* GetTileAtWorldPosition(Vec2f position);
  TileData* GetTileAtWorldPosition(float worldX, float worldY);
  Vec2 GetTileIndexFromWorldPosition(Vec2f position);
  Vec2 GetTileIndexFromWorldPosition(float worldX, float worldY);
  TileData* GetTileAtTileIndex(Vec2 tileIndex);
  TileData* GetTileAtTileIndex(int tileX, int tileY);

  // Building placement methods
  bool CanPlaceBuilding(Vec2 tileIndex, int width, int height) const;
  bool CanPlaceBuilding(int tileX, int tileY, int width, int height) const;
  void PlaceBuilding(EntityID entity, Vec2 tileIndex, int width, int height);
  void PlaceBuilding(EntityID entity, int tileX, int tileY, int width,
                     int height);
  void RemoveBuilding(EntityID entity, const std::vector<Vec2>& occupiedTiles);
  bool IsTileMovable(Vec2f worldPos);
  bool IsTileMovable(Vec2 tileIdx);

  const std::map<ChunkCoord, Chunk>& GetActiveChunks() const {return activeChunks;}
  inline rsrc_amt_t GetMinironOreAmount() const { return minironOreAmount; }
  inline rsrc_amt_t GetMaxironOreAmount() const { return maxironOreAmount; }

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
  rsrc_amt_t minironOreAmount;
  rsrc_amt_t maxironOreAmount = 10000;
  int viewDistance = 2;  // 플레이어로부터 몇 청크까지 로드할 것인가
};

#endif /* CORE_WORLD_ */
