#include "Core/World.h"

#include <cmath>
#include <iostream>
#include <map>
#include <random>

#include "Common.h"
#include "Components/BuildingComponent.h"
#include "Components/ChunkComponent.h"
#include "Components/InactiveComponent.h"
#include "Components/ResourceNodeComponent.h"
#include "Components/SpriteComponent.h"
#include "Components/TextComponent.h"
#include "Components/TransformComponent.h"
#include "Core/AssetManager.h"
#include "Core/Chunk.h"
#include "Core/Registry.h"
#include "Core/TileData.h"
#include "Core/Type.h"
#include "Core/World.h"
#include "FastNoiseLite.h"
#include "SDL_ttf.h"

World::World(SDL_Renderer* renderer, Registry* registry, TTF_Font* font)
    : font(font), renderer(renderer), registry(registry) {
  std::random_device rd;
  randomGenerator.seed(rd());
  distribution = std::normal_distribution<float>(0.0, 1.0);
}

void World::Update(EntityID player) {
  if (!registry->HasComponent<TransformComponent>(player)) return;

  const Vec2f playerPosition =
      registry->GetComponent<TransformComponent>(player).position;

  int playerChunkX = floor(playerPosition.x / (CHUNK_WIDTH * TILE_PIXEL_SIZE));
  int playerChunkY =
      floor(playerPosition.y / (CHUNK_HEIGHT * TILE_PIXEL_SIZE));

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

TileData* World::GetTileAtWorldPosition(Vec2f position) {
  return GetTileAtWorldPosition(position.x, position.y);
}

TileData* World::GetTileAtWorldPosition(float worldX, float worldY) {
  int tileX = static_cast<int>(worldX) / TILE_PIXEL_SIZE;
  int tileY = static_cast<int>(worldY) / TILE_PIXEL_SIZE;

  return GetTileAtTileIndex(tileX, tileY);
}

Vec2 World::GetTileIndexFromWorldPosition(Vec2f position) {
  return GetTileIndexFromWorldPosition(position.x, position.y);
}

Vec2 World::GetTileIndexFromWorldPosition(float worldX, float worldY) {
  int tileX = static_cast<int>(worldX) / TILE_PIXEL_SIZE;
  int tileY = static_cast<int>(worldY) / TILE_PIXEL_SIZE;

  return Vec2(tileX, tileY);
}

TileData* World::GetTileAtTileIndex(Vec2 tileIndex) {
  return GetTileAtTileIndex(tileIndex.x, tileIndex.y);
}

TileData* World::GetTileAtTileIndex(int tileX, int tileY) {
  int chunkX = tileX / CHUNK_WIDTH;
  int chunkY = tileY / CHUNK_HEIGHT;

  if (tileX < 0) chunkX -= 1;
  if (tileY < 0) chunkY -= 1;

  auto it = activeChunks.find({chunkX, chunkY});
  if (it != activeChunks.end()) {
    Vec2 localCoords = it->second.GetLocalTileIndex(tileX, tileY);
    return it->second.GetTile(localCoords.x, localCoords.y);
  }

  return nullptr;
}

const std::map<ChunkCoord, Chunk>& World::GetActiveChunks() const {
  return activeChunks;
}

bool World::CanPlaceBuilding(Vec2 tileIndex, int width, int height) const {
  return CanPlaceBuilding(tileIndex.x, tileIndex.y, width, height);
}

bool World::CanPlaceBuilding(int tileX, int tileY, int width, int height) const {
  // Check if all tiles for this building are available
  for (int dy = 0; dy < height; ++dy) {
    for (int dx = 0; dx < width; ++dx) {
      int checkX = tileX + dx;
      int checkY = tileY + dy;
      
      // Get tile data (const cast is safe here since we're only checking)
      TileData* tile = const_cast<World*>(this)->GetTileAtTileIndex(checkX, checkY);
      if (!tile) {
        return false; // Tile doesn't exist (chunk not loaded)
      }
      
      if (tile->occupyingEntity != INVALID_ENTITY) {
        return false; // Tile is already occupied
      }
      
      if (tile->type == TileType::Water || tile->type == TileType::Invalid) {
        return false; // Cannot build on water or invalid tiles
      }
    }
  }
  return true;
}

void World::PlaceBuilding(EntityID entity, Vec2 tileIndex, int width, int height) {
  PlaceBuilding(entity, tileIndex.x, tileIndex.y, width, height);
}

void World::PlaceBuilding(EntityID entity, int tileX, int tileY, int width, int height) {
  std::vector<Vec2> occupiedTiles;
  
  // Mark all tiles as occupied by this building
  for (int dy = 0; dy < height; ++dy) {
    for (int dx = 0; dx < width; ++dx) {
      int checkX = tileX + dx;
      int checkY = tileY + dy;
      
      TileData* tile = GetTileAtTileIndex(checkX, checkY);
      if (tile) {
        tile->occupyingEntity = entity;
        occupiedTiles.push_back({checkX, checkY});
      }
    }
  }
  
  // Store occupied tiles in the building component for cleanup
  if (registry->HasComponent<BuildingComponent>(entity)) {
    auto& building = registry->GetComponent<BuildingComponent>(entity);
    building.occupiedTiles = occupiedTiles;
  }
}

void World::RemoveBuilding(EntityID entity, const std::vector<Vec2>& occupiedTiles) {
  // Clear all tiles that this building occupied
  for (const Vec2& tileIndex : occupiedTiles) {
    TileData* tile = GetTileAtTileIndex(tileIndex);
    if (tile && tile->occupyingEntity == entity) {
      tile->occupyingEntity = INVALID_ENTITY;
    }
  }
}

void World::LoadChunk(int chunkX, int chunkY) {
  auto it = chunkCache.find({chunkX, chunkY});
  if (it != chunkCache.end()) {
    Chunk& chunk = it->second;
    // Reactivate entities
    registry->RemoveComponent<InactiveComponent>(chunk.chunkEntity);
    for (int y = 0; y < CHUNK_HEIGHT; ++y) {
      for (int x = 0; x < CHUNK_WIDTH; ++x) {
        TileData* tile = chunk.GetTile(x, y);
        if (tile && tile->occupyingEntity != INVALID_ENTITY) {
          registry->RemoveComponent<InactiveComponent>(tile->occupyingEntity);
        }
      }
    }
    activeChunks.insert({it->first, chunk});
    chunkCache.erase(it);
    std::cout << "Reloaded Chunk at (" << chunk.chunkX << ", " << chunk.chunkY
              << ")\n";
  } else {
    Chunk chunk(chunkX, chunkY);
    GenerateChunk(chunk);
    activeChunks.insert({{chunkX, chunkY}, chunk});
  }
}

void World::UnloadChunk(Chunk& chunk) {
  // Deactivate entities
  registry->EmplaceComponent<InactiveComponent>(chunk.chunkEntity);
  for (int y = 0; y < CHUNK_HEIGHT; ++y) {
    for (int x = 0; x < CHUNK_WIDTH; ++x) {
      TileData* tile = chunk.GetTile(x, y);
      if (tile && tile->occupyingEntity != INVALID_ENTITY) {
        registry->EmplaceComponent<InactiveComponent>(tile->occupyingEntity);
      }
    }
  }
  std::cout << "Unloaded Chunk at (" << chunk.chunkX << ", " << chunk.chunkY
            << ")\n";
}

SDL_Texture* World::CreateChunkTexture(Chunk& chunk) {
  // Create a texture for the entire chunk
  SDL_Texture* chunkTexture = SDL_CreateTexture(
      renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
      CHUNK_WIDTH * TILE_PIXEL_SIZE, CHUNK_HEIGHT * TILE_PIXEL_SIZE);

  // Set the texture as the render target
  SDL_SetRenderTarget(renderer, chunkTexture);

  // Clear the texture with transparent color
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  SDL_RenderClear(renderer);

  // Get the tileset texture for drawing individual tiles
  SDL_Texture* tilesetTexture = AssetManager::Instance().getTexture(
      "assets/img/tile/dirt-1.png", renderer);

  // Draw all tiles in the chunk to the texture
  for (int y = 0; y < CHUNK_HEIGHT; ++y) {
    for (int x = 0; x < CHUNK_WIDTH; ++x) {
      TileData* tile = chunk.GetTile(x, y);

      // Determine source rectangle based on tile type
      SDL_Rect srcRect;
      switch (tile->type) {
        case TileType::Dirt:
          srcRect = {0, 0, 64, 64};
          break;
        case TileType::Grass:
          srcRect = {64, 0, 64, 64};
          break;
        case TileType::Water:
          srcRect = {128, 0, 64, 64};
          break;
        case TileType::Stone:
          srcRect = {192, 0, 64, 64};
          break;
        default:
          break;
      }

      // Destination rectangle for this tile in the chunk texture
      SDL_Rect destRect = {x * TILE_PIXEL_SIZE, y * TILE_PIXEL_SIZE,
                           TILE_PIXEL_SIZE, TILE_PIXEL_SIZE};

      // Render the tile to the chunk texture
      SDL_RenderCopy(renderer, tilesetTexture, &srcRect, &destRect);

      // if (tile->debugValue > 1) {
      //   sprintf(text, "%.2f", tile->debugValue);

      //   SDL_Surface* textSurface =
      //       TTF_RenderText_Blended(font, text, SDL_Color{255, 255, 255});
      //   SDL_Texture* textTexture =
      //       SDL_CreateTextureFromSurface(renderer, textSurface);
      //   destRect.h = textSurface->h;
      //   destRect.w = textSurface->w;
      //   SDL_FreeSurface(textSurface);
      //   SDL_RenderCopy(renderer, textTexture, nullptr, &destRect);
      // }
    }
  }

  // Reset render target to default
  SDL_SetRenderTarget(renderer, nullptr);

  return chunkTexture;
}

void World::GenerateChunk(Chunk& chunk) {
  // TODO : camera lagged on chunk generation
  FastNoiseLite noise;
  noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
  noise.SetFrequency(0.05f);

  // terrain generation
  for (int y = 0; y < CHUNK_HEIGHT; ++y) {
    for (int x = 0; x < CHUNK_WIDTH; ++x) {
      int worldTileX = chunk.chunkX * CHUNK_WIDTH + x;
      int worldTileY = chunk.chunkY * CHUNK_HEIGHT + y;

      float terrainValue = noise.GetNoise((float)worldTileX, (float)worldTileY);
      TileData* tile = chunk.GetTile(x, y);

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
  // oreNoise.SetSeed(randomGenerator());
  for (int y = 0; y < CHUNK_HEIGHT; ++y) {
    for (int x = 0; x < CHUNK_WIDTH; ++x) {
      int worldTileX = chunk.chunkX * CHUNK_WIDTH + x;
      int worldTileY = chunk.chunkY * CHUNK_HEIGHT + y;

      float oreValue = oreNoise.GetNoise((float)worldTileX, (float)worldTileY);

      // TileData* tile = chunk.GetTile(x, y);

      if (oreValue > oreThreshold &&
          chunk.GetTile(x, y)->type != TileType::Water) {
        TileData* tile = chunk.GetTile(x, y);

        if (tile->occupyingEntity == INVALID_ENTITY) {
          EntityID oreNode = registry->CreateEntity();

          rsrc_amt_t oreAmount = static_cast<rsrc_amt_t>(
              static_cast<float>(maxironOreAmount) * oreValue);

          registry->AddComponent<TransformComponent>(
              oreNode,
              TransformComponent{
                  {static_cast<float>(worldTileX * TILE_PIXEL_SIZE),
                   static_cast<float>(worldTileY * TILE_PIXEL_SIZE)}});

          registry->AddComponent<ResourceNodeComponent>(
              oreNode, ResourceNodeComponent{oreAmount, OreType::Iron});

          TextComponent textComp;
          snprintf(textComp.text, sizeof(textComp.text), "init");
          textComp.color = SDL_Color{255, 255, 255, 255};
          registry->EmplaceComponent<TextComponent>(oreNode, textComp);

          SDL_Texture* spritesheet = AssetManager::Instance().getTexture(
              "assets/img/entity/iron-ore.png", renderer);
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
          tile->occupyingEntity = oreNode;
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
  chunkComp.chunkTexture = CreateChunkTexture(chunk);
  chunkComp.needsRedraw = false;
  registry->EmplaceComponent<ChunkComponent>(chunkEntity, chunkComp);

  std::cout << "Generated Chunk at (" << chunk.chunkX << ", " << chunk.chunkY
            << " id=" << chunkEntity << ")\n";
}
