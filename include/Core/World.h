#ifndef CORE_WORLD_
#define CORE_WORLD_

#include <map>
#include <random>

#include "Components/ResourceNodeComponent.h"
#include "Core/Chunk.h"
#include "Core/Entity.h"
#include "Core/TileData.h"
#include "Core/Type.h"
#include "SDL_ttf.h"

class Registry;
class WorldAssetManager;
class EventDispatcher;
class EntityFactory;

// TODO : asyncronized chunk generation : threads, iocp whatever

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
 * @details Handles the procedural generation of the world, loading and
 * unloading of chunks based on player proximity, and provides an interface for
 *          querying tile data and managing building placement.
 */
class World {
  TTF_Font* font;
  Registry* registry;
  WorldAssetManager* worldAssetManager;
  EventDispatcher* eventDispatcher;
  EntityFactory* factory;
  EntityID player;

 public:
  World(Registry* registry, WorldAssetManager* worldAssetManager,
        EntityFactory* factory, EventDispatcher* eventDispatcher,
        TTF_Font* font);
  ~World();

  /**
   * @brief Updates the world state, loading/unloading chunks around the player.
   * @param player The player entity, used to determine which chunks should be
   * active.
   */
  void Update();

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
  Vec2 GetTileIndexFromWorldPosition(Vec2f position) const;
  Vec2 GetTileIndexFromWorldPosition(float worldX, float worldY) const;

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
  bool HasNoOcuupyingEntity(Vec2 tileIndex, int width, int height);
  bool HasNoOcuupyingEntity(int tileX, int tileY, int width, int height);

  /**
   * @brief Marks tiles as occupied by a building.
   * @param entity The building entity.
   * @param tileIndex The top-left tile index for the building.
   * @param width The width of the building in tiles.
   * @param height The height of the building in tiles.
   */
  void OccupyTile(EntityID entity, Vec2 tileIndex, int width, int height);
  void OccupyTile(EntityID entity, int tileX, int tileY, int width,
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
  bool DoesTileBlockMovement(Vec2f worldPos);
  bool DoesTileBlockMovement(Vec2 tileIdx);

  void GeneratePlayer(Vec2f pos = {0.f,0.f});

  const std::map<ChunkCoord, Chunk>& GetActiveChunks() const {
    return activeChunks;
  }
  inline rsrc_amt_t GetMinironOreAmount() const { return minironOreAmount; }
  inline rsrc_amt_t GetMaxironOreAmount() const { return maxironOreAmount; }
  inline EntityID GetPlayer() const { return player; }
  inline int GetViewDistance() const { return viewDistance; }

 private:
  void LoadChunk(int chunkX, int chunkY);
  void GenerateChunk(Chunk& chunk);
  void UnloadChunk(Chunk& chunk);

  std::mt19937 randomGenerator;
  std::normal_distribution<float> distribution;
  std::map<ChunkCoord, Chunk> activeChunks;
  std::map<ChunkCoord, Chunk> chunkCache;
  rsrc_amt_t minironOreAmount;
  // TODO should be configurable
  rsrc_amt_t maxironOreAmount = 10000;
  // HACK should be changed with screen size
  int viewDistance = 2;  // Chunk load distance from player
};

#endif/* CORE_WORLD_ */
