#include "Core/World.h"

#include <cmath>
#include <iostream>
#include <map>
#include <random>

#include "Common.h"
#include "Components/BuildingComponent.h"
#include "Components/ChunkComponent.h"
#include "Components/DebugRectComponent.h"
#include "Components/InactiveComponent.h"
#include "Components/ResourceNodeComponent.h"
#include "Components/SpriteComponent.h"
#include "Components/TextComponent.h"
#include "Components/TransformComponent.h"
#include "Core/Chunk.h"
#include "Core/EntityFactory.h"
#include "Core/Event.h"
#include "Core/EventDispatcher.h"
#include "Core/Registry.h"
#include "Core/TileData.h"
#include "Core/Type.h"
#include "Core/World.h"
#include "Core/WorldAssetManager.h"
#include "FastNoiseLite.h"
#include "SDL_ttf.h"

World::World(Registry *registry, WorldAssetManager *worldAssetManager,
             EntityFactory *factory, EventDispatcher *eventDispatcher,
             TTF_Font *font, bool bIsServer)
    : registry(registry),
      factory(factory),
      worldAssetManager(worldAssetManager),
      eventDispatcher(eventDispatcher),
      font(font),
      localPlayer(INVALID_ENTITY),
      bIsServer(bIsServer) {
  std::random_device rd;
  randomGenerator.seed(rd());
  distribution = std::normal_distribution<float>(0.0, 1.0);
}

void World::Update() {
  if (!registry->HasComponent<TransformComponent>(localPlayer)) return;

  const Vec2f playerPosition =
      registry->GetComponent<TransformComponent>(localPlayer).position;

  int playerChunkX =
      std::floor(playerPosition.x / (CHUNK_WIDTH * TILE_PIXEL_SIZE));
  int playerChunkY =
      std::floor(playerPosition.y / (CHUNK_HEIGHT * TILE_PIXEL_SIZE));

  auto it = activeChunks.begin();
  while (it != activeChunks.end()) {
    int dx = std::abs(it->first.x - playerChunkX);
    int dy = std::abs(it->first.y - playerChunkY);

    // Unload far chunk
    if (dx > viewDistance || dy > viewDistance) {
      UnloadChunk(it->second);
      chunkCache.insert({it->first, it->second});
      it = activeChunks.erase(it);
    } else {
      ++it;
    }
  }

  // Load chunk in view dist
  for (int y = playerChunkY - viewDistance; y <= playerChunkY + viewDistance;
       ++y) {
    for (int x = playerChunkX - viewDistance; x <= playerChunkX + viewDistance;
         ++x) {
      if (activeChunks.find({x, y}) == activeChunks.end()) {
        LoadChunk(x, y);
      }
    }
  }
}

void World::GeneratePlayer(clientid_t clientID, Vec2f pos, bool bIsLocal) {
  EntityID player = factory->CreatePlayer(this, pos, clientID, bIsLocal);
  if (bIsLocal) 
    localPlayer = player;
  clientPlayerMap[clientID] = player;
}

TileData *World::GetTileAtWorldPosition(Vec2f position) {
  return GetTileAtWorldPosition(position.x, position.y);
}

TileData *World::GetTileAtWorldPosition(float worldX, float worldY) {
  return GetTileAtTileIndex(GetTileIndexFromWorldPosition(worldX, worldY));
}

Vec2 World::GetTileIndexFromWorldPosition(Vec2f position) const {
  return GetTileIndexFromWorldPosition(position.x, position.y);
}

Vec2 World::GetTileIndexFromWorldPosition(float worldX, float worldY) const {
  int tileX = std::floor(worldX / TILE_PIXEL_SIZE);
  int tileY = std::floor(worldY / TILE_PIXEL_SIZE);

  return Vec2(tileX, tileY);
}

TileData *World::GetTileAtTileIndex(Vec2 tileIndex) {
  return GetTileAtTileIndex(tileIndex.x, tileIndex.y);
}

TileData *World::GetTileAtTileIndex(int tileX, int tileY) {
  int chunkX = std::floor(static_cast<float>(tileX) / CHUNK_WIDTH);
  int chunkY = std::floor(static_cast<float>(tileY) / CHUNK_HEIGHT);

  auto it = activeChunks.find({chunkX, chunkY});
  if (it != activeChunks.end()) {
    Vec2 localCoords = it->second.GetLocalTileIndex(tileX, tileY);
    return it->second.GetTile(localCoords.x, localCoords.y);
  }

  return nullptr;
}

bool World::IsTilePassable(Vec2f worldPos) {
  return IsTilePassable(GetTileIndexFromWorldPosition(worldPos));
}
bool World::IsTilePassable(Vec2 tileIdx) {
  TileData *tile = GetTileAtTileIndex(tileIdx);
  if (tile->type == TileType::Water || tile->type == TileType::Invalid)
    return false;
  if (tile->occupyingEntity != INVALID_ENTITY &&
      registry->HasComponent<BuildingComponent>(tile->occupyingEntity))
    return false;

  return true;
}

bool World::HasNoOcuupyingEntity(Vec2 tileIndex, int width, int height) {
  return HasNoOcuupyingEntity(tileIndex.x, tileIndex.y, width, height);
}

bool World::HasNoOcuupyingEntity(int tileX, int tileY, int width, int height) {
  // Check if all tiles for this building are available
  for (int dy = 0; dy < height; ++dy) {
    for (int dx = 0; dx < width; ++dx) {
      int checkX = tileX + dx;
      int checkY = tileY + dy;

      // Get tile data (const cast is safe here since we're only checking)
      TileData *tile =
          const_cast<World *>(this)->GetTileAtTileIndex(checkX, checkY);
      if (!tile) {
        return false;  // Tile doesn't exist (chunk not loaded)
      }

      if (tile->occupyingEntity != INVALID_ENTITY) {
        return false;  // Tile is already occupied
      }

      if (tile->type == TileType::Water || tile->type == TileType::Invalid) {
        return false;  // Cannot build on water or invalid tiles
      }

      auto &playertrans =
          registry->GetComponent<TransformComponent>(localPlayer);
      if (tile == GetTileAtWorldPosition(playertrans.position)) return false;
    }
  }
  return true;
}

void World::OccupyTile(EntityID entity, Vec2 tileIndex, int width, int height) {
  OccupyTile(entity, tileIndex.x, tileIndex.y, width, height);
}

void World::OccupyTile(EntityID entity, int tileX, int tileY, int width,
                       int height) {
  std::vector<Vec2> occupiedTiles;

  // Mark all tiles as occupied by this building
  for (int dy = 0; dy < height; ++dy) {
    for (int dx = 0; dx < width; ++dx) {
      int checkX = tileX + dx;
      int checkY = tileY + dy;

      TileData *tile = GetTileAtTileIndex(checkX, checkY);
      if (tile) {
        tile->occupyingEntity = entity;
        occupiedTiles.push_back({checkX, checkY});
      }
    }
  }

  // Store occupied tiles in the building component for cleanup
  if (registry->HasComponent<BuildingComponent>(entity)) {
    auto &building = registry->GetComponent<BuildingComponent>(entity);
    building.occupiedTiles = occupiedTiles;
  }
}

void World::RemoveBuilding(EntityID entity,
                           const std::vector<Vec2> &occupiedTiles) {
  // Clear all tiles that this building occupied
  for (const Vec2 &tileIndex : occupiedTiles) {
    TileData *tile = GetTileAtTileIndex(tileIndex);
    if (tile && tile->occupyingEntity == entity) {
      tile->occupyingEntity = INVALID_ENTITY;
    }
  }
}

void World::LoadChunk(int chunkX, int chunkY) {
  auto it = chunkCache.find({chunkX, chunkY});
  if (it != chunkCache.end()) {
    Chunk &chunk = it->second;
    // Reactivate entities
    registry->RemoveComponent<InactiveComponent>(chunk.chunkEntity);
    for (int y = 0; y < CHUNK_HEIGHT; ++y) {
      for (int x = 0; x < CHUNK_WIDTH; ++x) {
        TileData *tile = chunk.GetTile(x, y);
        if (tile) {
          if (tile->occupyingEntity != INVALID_ENTITY)
            registry->RemoveComponent<InactiveComponent>(tile->occupyingEntity);
          if (tile->oreEntity != INVALID_ENTITY)
            registry->RemoveComponent<InactiveComponent>(tile->oreEntity);
        }
      }
    }
    activeChunks.insert({it->first, chunk});
    chunkCache.erase(it);
    // std::cout << "Reloaded Chunk at (" << chunk.chunkX << ", " <<
    // chunk.chunkY << ")\n";
  } else {
    Chunk chunk(chunkX, chunkY);
    GenerateChunk(chunk);
    activeChunks.insert({{chunkX, chunkY}, chunk});
  }
}

void World::UnloadChunk(Chunk &chunk) {
  // Deactivate entities
  registry->EmplaceComponent<InactiveComponent>(chunk.chunkEntity);
  for (int y = 0; y < CHUNK_HEIGHT; ++y) {
    for (int x = 0; x < CHUNK_WIDTH; ++x) {
      TileData *tile = chunk.GetTile(x, y);
      if (tile) {
        if (tile->occupyingEntity != INVALID_ENTITY)
          registry->EmplaceComponent<InactiveComponent>(tile->occupyingEntity);
        if (tile->oreEntity != INVALID_ENTITY)
          registry->EmplaceComponent<InactiveComponent>(tile->oreEntity);
      }
    }
  }
  // std::cout << "Unloaded Chunk at (" << chunk.chunkX << ", " << chunk.chunkY
  // << ")\n";
}

void World::GenerateChunk(Chunk &chunk) {
  FastNoiseLite noise;
  noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
  noise.SetFrequency(0.05f);
  int cx = chunk.chunkX;
  int cy = chunk.chunkY;
  EntityID chunkDebugRect = registry->CreateEntity();
#ifdef DRAW_DEBUG_RECTS
  registry->EmplaceComponent<DebugRectComponent>(
      chunkDebugRect,
      DebugRectComponent{0, 0, TILE_PIXEL_SIZE * CHUNK_WIDTH,
                         TILE_PIXEL_SIZE * CHUNK_HEIGHT, 255, 0, 0, 255});
  registry->EmplaceComponent<TransformComponent>(
      chunkDebugRect,
      TransformComponent{
          {(float)(chunk.chunkX * CHUNK_WIDTH * TILE_PIXEL_SIZE),
           float(chunk.chunkY * CHUNK_HEIGHT * TILE_PIXEL_SIZE)}});

  TextComponent textComp;
  snprintf(textComp.text, sizeof(textComp.text), "chunk %d:%d", chunk.chunkX,
           chunk.chunkY);
  textComp.color = SDL_Color{255, 255, 255, 255};
  registry->EmplaceComponent<TextComponent>(chunkDebugRect, textComp);
#endif
  // terrain generation
  for (int y = 0; y < CHUNK_HEIGHT; ++y) {
    for (int x = 0; x < CHUNK_WIDTH; ++x) {
      int worldTileX = chunk.chunkX * CHUNK_WIDTH + x;
      int worldTileY = chunk.chunkY * CHUNK_HEIGHT + y;
#ifdef DRAW_DEBUG_RECTS
      EntityID tileDebugRect = registry->CreateEntity();
      registry->EmplaceComponent<DebugRectComponent>(
          tileDebugRect, DebugRectComponent{0, 0, TILE_PIXEL_SIZE,
                                            TILE_PIXEL_SIZE, 0, 255, 0, 80});
      registry->EmplaceComponent<TransformComponent>(
          tileDebugRect,
          TransformComponent{{(float)(worldTileX * TILE_PIXEL_SIZE),
                              (float)(worldTileY * TILE_PIXEL_SIZE)}});

      TextComponent textComp;
      snprintf(textComp.text, sizeof(textComp.text), "tile %d:%d", worldTileX,
               worldTileY);
      textComp.color = SDL_Color{255, 255, 255, 255};
      registry->EmplaceComponent<TextComponent>(tileDebugRect, textComp);
#endif
      float terrainValue = noise.GetNoise((float)worldTileX, (float)worldTileY);
      TileData *tile = chunk.GetTile(x, y);

      if (terrainValue < -0.2f) {
        tile->type = TileType::Water;
      } else if (terrainValue < 0.3f) {
        tile->type = TileType::Dirt;
      } else {
        tile->type = TileType::Grass;
      }
    }
  }

  // ore group generation
  FastNoiseLite oreNoise;
  oreNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
  oreNoise.SetFrequency(0.02f);
  float oreThreshold = 0.5f;
  minironOreAmount = oreThreshold * static_cast<float>(maxironOreAmount);
  for (int y = 0; y < CHUNK_HEIGHT; ++y) {
    for (int x = 0; x < CHUNK_WIDTH; ++x) {
      int worldTileX = chunk.chunkX * CHUNK_WIDTH + x;
      int worldTileY = chunk.chunkY * CHUNK_HEIGHT + y;

      float oreValue = oreNoise.GetNoise((float)worldTileX, (float)worldTileY);

      if (oreValue > oreThreshold &&
          chunk.GetTile(x, y)->type != TileType::Water) {
        TileData *tile = chunk.GetTile(x, y);

        if (tile->occupyingEntity == INVALID_ENTITY) {
          EntityID oreNode = registry->CreateEntity();

          rsrc_amt_t oreAmount = static_cast<rsrc_amt_t>(
              static_cast<float>(maxironOreAmount) * oreValue);

          registry->EmplaceComponent<TransformComponent>(
              oreNode, TransformComponent{
                           {static_cast<float>(worldTileX * TILE_PIXEL_SIZE),
                            static_cast<float>(worldTileY * TILE_PIXEL_SIZE)}});

          registry->EmplaceComponent<ResourceNodeComponent>(
              oreNode, ResourceNodeComponent{oreAmount, OreType::Iron});

          TextComponent textComp;
          snprintf(textComp.text, sizeof(textComp.text), "%d %d", worldTileX,
                   worldTileY);
          textComp.color = SDL_Color{255, 255, 255, 255};
          registry->EmplaceComponent<TextComponent>(oreNode, textComp);

          SDL_Texture *spritesheet =
              worldAssetManager->getTexture("assets/img/entity/iron-ore.png");
          SpriteComponent spriteComp;
          spriteComp.texture = spritesheet;
          // tile->debugValue = oreAmount;

          int richnessIndex =
              (IRON_SPRITESHEET_HEIGHT - 1) -
              std::min(
                  7.0f,
                  std::floor(
                      static_cast<float>(oreAmount - minironOreAmount) /
                      static_cast<float>(maxironOreAmount - minironOreAmount) *
                      8.f));
          spriteComp.srcRect = {0, richnessIndex * 128, 128, 128};
          spriteComp.renderRect = {0, 0, TILE_PIXEL_SIZE, TILE_PIXEL_SIZE};
          registry->EmplaceComponent<SpriteComponent>(oreNode, spriteComp);
          tile->oreEntity = oreNode;
          tile->type = TileType::Stone;
        }
      }
    }
  }

  // Create a single entity for the entire chunk with a pre-rendered texture
  EntityID chunkEntity = registry->CreateEntity();
  chunk.chunkEntity = chunkEntity;

  // Calculate world position of the chunk (top-left corner)
  float worldX =
      static_cast<float>(chunk.chunkX * CHUNK_WIDTH * TILE_PIXEL_SIZE);
  float worldY =
      static_cast<float>(chunk.chunkY * CHUNK_HEIGHT * TILE_PIXEL_SIZE);

  // Add transform component for positioning
  registry->EmplaceComponent<TransformComponent>(
      chunkEntity, TransformComponent{{worldX, worldY}});

  // Create and add the chunk component with the pre-rendered texture
  ChunkComponent chunkComp;
  chunkComp.chunkTexture = worldAssetManager->CreateChunkTexture(chunk);
  chunkComp.bNeedsRedraw = false;
  registry->EmplaceComponent<ChunkComponent>(chunkEntity, chunkComp);

  // std::cout << "Generated Chunk at (" << chunk.chunkX << ", " << chunk.chunkY
  // << " id=" << chunkEntity << ")\n";
}

World::~World() = default;