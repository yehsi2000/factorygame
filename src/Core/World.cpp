#include "Core/World.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <random>

#include "Components/ResourceNodeComponent.h"
#include "Components/SpriteComponent.h"
#include "Components/TextComponent.h"
#include "Components/TransformComponent.h"
#include "Core/AssetManager.h"
#include "Core/Chunk.h"
#include "Core/GEngine.h"
#include "Core/Registry.h"
#include "Core/TileData.h"
#include "Core/Type.h"
#include "Lib/FastNoiseLite.h"
#include "SDL.h"
#include "SDL_ttf.h"

World::World(GEngine* engine) : engine(engine) {
  std::random_device rd;
  randomGenerator.seed(rd());
  distribution = std::normal_distribution<float>(0.0, 1.0);
}

void World::Update(Vec2f playerPosition) {
  // 플레이어가 있는 청크 좌표 계산
  int playerChunkX = floor(playerPosition.x / (CHUNK_WIDTH * TILE_PIXEL_WIDTH));
  int playerChunkY =
      floor(playerPosition.y / (CHUNK_HEIGHT * TILE_PIXEL_HEIGHT));

  // 현재 로드된 청크들을 확인하고, 필요 없는 것은 언로드
  auto it = activeChunks.begin();
  while (it != activeChunks.end()) {
    int dx = std::abs(it->first.x - playerChunkX);
    int dy = std::abs(it->first.y - playerChunkY);

    // 시야 거리보다 멀리 떨어진 청크는 언로드
    if (dx > viewDistance || dy > viewDistance) {
      UnloadChunk(it->second);
      it = activeChunks.erase(it);
    } else {
      ++it;
    }
  }

  // 시야 거리 내의 청크 로드
  for (int y = playerChunkY - viewDistance; y <= playerChunkY + viewDistance;
       ++y) {
    for (int x = playerChunkX - viewDistance; x <= playerChunkX + viewDistance;
         ++x) {
      // 아직 로드되지 않은 청크라면 로드
      if (activeChunks.find({x, y}) == activeChunks.end()) {
        LoadChunk(x, y);
      }
    }
  }
}

TileData* World::GetTileAtWorldPosition(float worldX, float worldY) {
  int tileX = static_cast<int>(worldX) / TILE_PIXEL_WIDTH;
  int tileY = static_cast<int>(worldY) / TILE_PIXEL_HEIGHT;

  return GetTileAtTileCoords(tileX, tileY);
}

TileData* World::GetTileAtTileCoords(int tileX, int tileY) {
  int chunkX = tileX / CHUNK_WIDTH;
  int chunkY = tileY / CHUNK_HEIGHT;

  // 음수일 경우 보정
  if (tileX < 0) chunkX -= 1;
  if (tileY < 0) chunkY -= 1;

  auto it = activeChunks.find({chunkX, chunkY});
  if (it != activeChunks.end()) {
    Vec2 localCoords = it->second.GetLocalTileCoords(tileX, tileY);
    return it->second.GetTile(localCoords.x, localCoords.y);
  }

  return nullptr;
}

const std::map<ChunkCoord, Chunk>& World::GetActiveChunks() const {
  return activeChunks;
}

void World::LoadChunk(int chunkX, int chunkY) {
  Chunk chunk(chunkX, chunkY);
  GenerateChunk(chunk);
  activeChunks.insert({{chunkX, chunkY}, chunk});
}

void World::UnloadChunk(Chunk& chunk) {
  for (int y = 0; y < CHUNK_HEIGHT; ++y) {
    for (int x = 0; x < CHUNK_WIDTH; ++x) {
      TileData* tile = chunk.GetTile(x, y);
      if (tile && tile->occupyingEntity != 0) {
        engine->GetRegistry()->DestroyEntity(tile->occupyingEntity);
        tile->occupyingEntity = 0;
      }
    }
  }
  std::cout << "Unloaded Chunk at (" << chunk.chunkX << ", " << chunk.chunkY
            << ")\n";
}

SDL_Texture* World::CreateChunkTexture(Chunk& chunk) {
  SDL_Renderer* renderer = engine->GetRenderer();
  // Create a texture for the entire chunk
  SDL_Texture* chunkTexture = SDL_CreateTexture(
      renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
      CHUNK_WIDTH * TILE_PIXEL_WIDTH, CHUNK_HEIGHT * TILE_PIXEL_HEIGHT);

  // Set the texture as the render target
  SDL_SetRenderTarget(renderer, chunkTexture);

  // Clear the texture with transparent color
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  SDL_RenderClear(renderer);

  // Get the tileset texture for drawing individual tiles
  SDL_Texture* tilesetTexture = AssetManager::getInstance().getTexture(
      "assets/img/tile/dirt-1.png", renderer);

  char* text = new char[10];
  // Draw all tiles in the chunk to the texture
  for (int y = 0; y < CHUNK_HEIGHT; ++y) {
    for (int x = 0; x < CHUNK_WIDTH; ++x) {
      TileData* tile = chunk.GetTile(x, y);

      // Determine source rectangle based on tile type
      SDL_Rect srcRect;
      switch (tile->type) {
        case TileType::tDirt:
          srcRect = {0, 0, 64, 64};
          break;
        case TileType::tGrass:
          srcRect = {64, 0, 64, 64};
          break;
        case TileType::tWater:
          srcRect = {128, 0, 64, 64};
          break;
        case TileType::tStone:
          srcRect = {192, 0, 64, 64};
          break;
      }

      // Destination rectangle for this tile in the chunk texture
      SDL_Rect destRect = {x * TILE_PIXEL_WIDTH, y * TILE_PIXEL_HEIGHT,
                           TILE_PIXEL_WIDTH, TILE_PIXEL_HEIGHT};

      // Render the tile to the chunk texture
      SDL_RenderCopy(renderer, tilesetTexture, &srcRect, &destRect);

      if (tile->debugValue > 1) {
        sprintf(text, "%.2f", tile->debugValue);

        SDL_Surface* textSurface = TTF_RenderText_Blended(
            engine->GetFont(), text, SDL_Color{255, 255, 255});
        SDL_Texture* textTexture =
            SDL_CreateTextureFromSurface(renderer, textSurface);
        destRect.h = textSurface->h;
        destRect.w = textSurface->w;
        SDL_FreeSurface(textSurface);
        SDL_RenderCopy(renderer, textTexture, nullptr, &destRect);
      }
    }
  }

  // Reset render target to default
  SDL_SetRenderTarget(renderer, nullptr);

  delete[] text;

  return chunkTexture;
}

void World::GenerateChunk(Chunk& chunk) {
  //TODO : 청크 생성 너무 빨리하면 카메라가 플레이어 이동 못따라가는 현상 발생
  FastNoiseLite noise;
  noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
  noise.SetFrequency(0.05f);  // 맵 스케일 조절

  // 1. 지형 생성
  for (int y = 0; y < CHUNK_HEIGHT; ++y) {
    for (int x = 0; x < CHUNK_WIDTH; ++x) {
      int worldTileX = chunk.chunkX * CHUNK_WIDTH + x;
      int worldTileY = chunk.chunkY * CHUNK_HEIGHT + y;

      float terrainValue = noise.GetNoise((float)worldTileX, (float)worldTileY);
      TileData* tile = chunk.GetTile(x, y);

      if (terrainValue < -0.2f) {
        tile->type = TileType::tWater;
      } else if (terrainValue < 0.3f) {
        tile->type = TileType::tDirt;
      } else {
        tile->type = TileType::tGrass;
      }
    }
  }

  // 2. 자원(Ore) 군집 생성
  FastNoiseLite oreNoise;
  Registry* registry = engine->GetRegistry();
  SDL_Renderer* renderer = engine->GetRenderer();
  TTF_Font* font = engine->GetFont();
  oreNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
  oreNoise.SetFrequency(0.05f);
  // oreNoise.SetSeed(randomGenerator());

  for (int y = 0; y < CHUNK_HEIGHT; ++y) {
    for (int x = 0; x < CHUNK_WIDTH; ++x) {
      int worldTileX = chunk.chunkX * CHUNK_WIDTH + x;
      int worldTileY = chunk.chunkY * CHUNK_HEIGHT + y;

      float oreValue = oreNoise.GetNoise((float)worldTileX, (float)worldTileY);
      float oreThreshold = 0.5f;
      TileData* tile = chunk.GetTile(x, y);
      if (oreValue > oreThreshold &&
          chunk.GetTile(x, y)->type != TileType::tWater) {
        TileData* tile = chunk.GetTile(x, y);
        if (tile->occupyingEntity == 0) {
          EntityID oreNode = registry->CreateEntity();
          float max_oreAmount = 10000.0;
          rsrc_amt_t oreAmount = max_oreAmount * oreValue;

          registry->AddComponent<TransformComponent>(
              oreNode, TransformComponent{
                           static_cast<float>(worldTileX * TILE_PIXEL_WIDTH),
                           static_cast<float>(worldTileY * TILE_PIXEL_HEIGHT)});
          registry->AddComponent<ResourceNodeComponent>(
              oreNode, ResourceNodeComponent{static_cast<rsrc_amt_t>(oreAmount),
                                             OreType::Iron});

          SDL_Texture* spritesheet = AssetManager::getInstance().getTexture(
              "assets/img/tile/iron-ore.png", renderer);
          SpriteComponent spriteComp;
          spriteComp.texture = spritesheet;
          tile->debugValue = oreAmount;
          rsrc_amt_t min_oreAmount = oreThreshold * max_oreAmount;

          int richnessIndex =
              7 -
              std::min(7.0, std::floor((double)(oreAmount - min_oreAmount) /
                                       (double)(max_oreAmount - min_oreAmount) *
                                       8.0));
          spriteComp.srcRect = {0, richnessIndex * 128, 128, 128};
          spriteComp.renderRect = {0, 0, TILE_PIXEL_WIDTH, TILE_PIXEL_HEIGHT};
          registry->EmplaceComponent<SpriteComponent>(oreNode, spriteComp);
          tile->occupyingEntity = oreNode;
          tile->type = TileType::tStone;
        }
      }
    }
  }

  // Create a single entity for the entire chunk with a pre-rendered texture
  EntityID chunkEntity = registry->CreateEntity();

  // Calculate world position of the chunk (top-left corner)
  float worldX =
      static_cast<float>(chunk.chunkX * CHUNK_WIDTH * TILE_PIXEL_WIDTH);
  float worldY =
      static_cast<float>(chunk.chunkY * CHUNK_HEIGHT * TILE_PIXEL_HEIGHT);

  // Add transform component for positioning
  registry->EmplaceComponent<TransformComponent>(
      chunkEntity, TransformComponent{worldX, worldY});

  // Create and add the chunk component with the pre-rendered texture
  ChunkComponent chunkComp;
  chunkComp.chunkTexture = CreateChunkTexture(chunk);
  chunkComp.needsRedraw = false;
  registry->EmplaceComponent<ChunkComponent>(chunkEntity, chunkComp);

  // Store the chunk entity ID in the chunk for later reference
  // We'll need to modify the Chunk class to store this if we want to update it
  // later

  std::cout << "Loaded Chunk at (" << chunk.chunkX << ", " << chunk.chunkY
            << ")\n";
}
