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

/**
 * @brief ChunkCoordinate operator for RB-tree comparison
 * 
 */
struct ChunkCoord {
  int x, y;
  bool operator<(const ChunkCoord& other) const {
    if (y < other.y) return true;
    if (y > other.y) return false;
    return x < other.x;
  }
};

/**
 * @brief Manages the game world, including chunk loading and tile data.
 * @details Handles the procedural generation of the world, loading and unloading
 *          of chunks based on player proximity, and provides an interface for
 *          querying tile data and managing building placement.
 */
class World {
  friend class TestWorld;

 public:
  World(SDL_Renderer* renderer, Registry* registry, TTF_Font* font);

  /**
   * @brief Updates the world state, loading/unloading chunks around the player.
   * @param player The player entity, used to determine which chunks should be active.
   */
  void Update(EntityID player);

  /**
   * @brief Gets the tile data at a specific world position.
   * @param position The world coordinates (in pixels).
   * @return A pointer to the TileData, or nullptr if the chunk is not loaded.
   */
  TileData* GetTileAtWorldPosition(Vec2f position);
  TileData* GetTileAtWorldPosition(float worldX, float worldY);

  /**
   * @brief Converts world coordinates to tile grid indices.
   * @param position The world coordinates (in pixels).
   * @return The corresponding tile index (e.g., [10, 25]).
   */
  Vec2 GetTileIndexFromWorldPosition(Vec2f position);
  Vec2 GetTileIndexFromWorldPosition(float worldX, float worldY);

  /**
   * @brief Gets the tile data at a specific tile grid index.
   * @param tileIndex The tile coordinates.
   * @return A pointer to the TileData, or nullptr if the chunk is not loaded.
   */
  TileData* GetTileAtTileIndex(Vec2 tileIndex);
  TileData* GetTileAtTileIndex(int tileX, int tileY);

  /**
   * @brief Checks if a building can be placed at a given location.
   * @param tileIndex The top-left tile index for the building.
   * @param width The width of the building in tiles.
   * @param height The height of the building in tiles.
   * @return True if the area is clear and buildable, false otherwise.
   */
  bool CanPlaceBuilding(Vec2 tileIndex, int width, int height) const;
  bool CanPlaceBuilding(int tileX, int tileY, int width, int height) const;

  /**
   * @brief Marks tiles as occupied by a building.
   * @param entity The building entity.
   * @param tileIndex The top-left tile index for the building.
   * @param width The width of the building in tiles.
   * @param height The height of the building in tiles.
   */
  void PlaceBuilding(EntityID entity, Vec2 tileIndex, int width, int height);
  void PlaceBuilding(EntityID entity, int tileX, int tileY, int width,
                     int height);
  
  /**
   * @brief Frees tiles previously occupied by a building.
   * @param entity The building entity being removed.
   * @param occupiedTiles A list of tile indices the building occupied.
   */
  void RemoveBuilding(EntityID entity, const std::vector<Vec2>& occupiedTiles);

  /**
   * @brief Checks if a tile at a given position is passable.
   * @param worldPos The world coordinates of the tile.
   * @return True if the tile is movable, false otherwise.
   */
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
  int viewDistance = 2;  // Chunk load distance from player
};

#endif/* CORE_WORLD_ */
