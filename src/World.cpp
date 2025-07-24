#include "World.h"

#include <cmath>
#include <iostream>

#include "AssetManager.h"
#include "Components/ResourceNodeComponent.h"
#include "Components/SpriteComponent.h"
#include "Components/TransformComponent.h"
#include "FastNoiseLite.h"
#include "Registry.h"
#include "SDL.h"
#include "TileData.h"
#include "Type.h"

World::World(Registry* _registry, SDL_Renderer* _renderer) {
  registry = _registry;
  renderer = _renderer;
}

void World::Update(Vec2f playerPosition) {
  // TODO: 텍스쳐 병합으로 월드배경을 하나만 만드는거 고려해보기
  // 플레이어가 있는 청크 좌표 계산
  int playerChunkX = playerPosition.x / (CHUNK_WIDTH * TILE_PIXEL_WIDTH);
  int playerChunkY = playerPosition.y / (CHUNK_HEIGHT * TILE_PIXEL_HEIGHT);

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
        registry->DestroyEntity(tile->occupyingEntity);
        tile->occupyingEntity = 0;
      }
    }
  }
  std::cout << "Unloaded Chunk at (" << chunk.chunkX << ", " << chunk.chunkY
            << ")\n";
}

void World::GenerateChunk(Chunk& chunk) {
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
  // FastNoiseLite oreNoise;
  // oreNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
  // oreNoise.SetFrequency(0.1f);
  // oreNoise.SetSeed(1337);  // 다른 시드 사용

  // for (int y = 0; y < CHUNK_HEIGHT; ++y) {
  //   for (int x = 0; x < CHUNK_WIDTH; ++x) {
  //     int worldTileX = chunk.chunkX * CHUNK_WIDTH + x;
  //     int worldTileY = chunk.chunkY * CHUNK_HEIGHT + y;

  //     float oreValue = oreNoise.GetNoise((float)worldTileX, (float)worldTileY);

  //     // 특정 임계값 이상이고, 땅 위에 있을 때만 자원 생성
  //     if (oreValue > 0.7f && chunk.GetTile(x, y)->type != TileType::tWater) {
  //       TileData* tile = chunk.GetTile(x, y);
  //       if (tile->occupyingEntity == 0) {  // 이미 다른 것이 없다면
  //         // Ore 엔티티 생성
  //         EntityID oreNode = registry->CreateEntity();
  //         // 월드 좌표 계산 (타일 중앙으로)
  //         int worldX = worldTileX * TILE_PIXEL_WIDTH + TILE_PIXEL_WIDTH / 2;
  //         int worldY = worldTileY * TILE_PIXEL_HEIGHT + TILE_PIXEL_HEIGHT / 2;

  //         registry->AddComponent<TransformComponent>(
  //             oreNode, TransformComponent{worldX, worldY});
  //         registry->AddComponent<ResourceNodeComponent>(
  //             oreNode,
  //             ResourceNodeComponent{100000, OreType::Iron});  // 자원 정보

  //         // 스프라이트 컴포넌트 추가 (64x64 타일 크기)
  //         SDL_Texture* spritesheet = AssetManager::getInstance().getTexture(
  //             "assets/img/tile/dirt-1.png",
  //             renderer);  // renderer will be set later
  //         SpriteComponent spriteComp;
  //         spriteComp.texture = spritesheet;
  //         spriteComp.srcRect = {
  //             64, 0, 64,
  //             64};  // 예시로 두 번째 타일 (64,0)에서 시작하는 64x64 영역
  //         registry->EmplaceComponent<SpriteComponent>(oreNode, spriteComp);

  //         // 타일에 생성된 엔티티 ID 기록
  //         tile->occupyingEntity = oreNode;
  //         tile->type = TileType::tStone;  // 자원 타일은 돌 배경으로
  //       }
  //     }
  //   }
  // }

  // 타일에 대한 스프라이트 엔티티 생성 (렌더링을 위해)
  for (int y = 0; y < CHUNK_HEIGHT; ++y) {
    for (int x = 0; x < CHUNK_WIDTH; ++x) {
      TileData* tile = chunk.GetTile(x, y);
      // 자원 노드가 없는 타일에 대해서만 타일 스프라이트 생성
      if (tile->occupyingEntity == 0) {
        EntityID tileEntity = registry->CreateEntity();
        int worldTileX = chunk.chunkX * CHUNK_WIDTH + x;
        int worldTileY = chunk.chunkY * CHUNK_HEIGHT + y;

        // 월드 좌표 계산 (타일 좌상단)
        float worldX = static_cast<float>(worldTileX) * TILE_PIXEL_WIDTH;
        float worldY = static_cast<float>(worldTileY) * TILE_PIXEL_HEIGHT;
        auto tfcomp = TransformComponent{worldX, worldY};
        registry->EmplaceComponent<TransformComponent>(
            tileEntity, tfcomp);

        // 스프라이트 컴포넌트 추가
        SDL_Texture* spritesheet = AssetManager::getInstance().getTexture(
            "assets/img/tile/dirt-1.png",
            renderer);  // renderer will be set later
        SpriteComponent spriteComp;
        spriteComp.texture = spritesheet;

        // 타일 타입에 따라 스프라이트 시트에서 다른 위치를 사용
        switch (tile->type) {
          case TileType::tDirt:
            spriteComp.srcRect = {
                0, 0, 64, 64};  // 첫 번째 타일 (0,0)에서 시작하는 64x64 영역
            break;
          case TileType::tGrass:
            spriteComp.srcRect = {64, 0, 64, 64};  // 두 번째 타일
            break;
          case TileType::tWater:
            spriteComp.srcRect = {128, 0, 64, 64};  // 세 번째 타일
            break;
          case TileType::tStone:
            spriteComp.srcRect = {192, 0, 64, 64};  // 네 번째 타일
            break;
        }

        registry->EmplaceComponent<SpriteComponent>(tileEntity, spriteComp);
        tile->occupyingEntity = tileEntity;
      }
    }
  }
  std::cout<<"Loaded Chunk at ("<<chunk.chunkX<<", "<<chunk.chunkY<<")\n";
}
