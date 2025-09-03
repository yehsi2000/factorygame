#include "Core/EntityFactory.h"

#include "Components/AnimationComponent.h"
#include "Components/AssemblingMachineComponent.h"
#include "Components/BuildingComponent.h"
#include "Components/InventoryComponent.h"
#include "Components/MiningDrillComponent.h"
#include "Components/MovableComponent.h"
#include "Components/MovementComponent.h"
#include "Components/PlayerStateComponent.h"
#include "Components/SpriteComponent.h"
#include "Components/TransformComponent.h"
#include "Core/AssetManager.h"
#include "Core/Registry.h"
#include "Core/TileData.h"
#include "Core/World.h"
#include "SDL.h"
#include "Util/AnimUtil.h"

EntityFactory::EntityFactory(Registry *registry, AssetManager *assetManager)
    : registry(registry), assetManager(assetManager) {}

EntityID EntityFactory::CreateAssemblingMachine(World *world, Vec2f worldPos) {
  if (registry == nullptr || world == nullptr) return INVALID_ENTITY;

  Vec2 tileIndex = world->GetTileIndexFromWorldPosition(worldPos);
  return CreateAssemblingMachine(world, tileIndex);
}

EntityID EntityFactory::CreateAssemblingMachine(World *world, Vec2 tileIndex) {
  if (registry == nullptr || world == nullptr) return INVALID_ENTITY;

  if (!world->CanPlaceBuilding(tileIndex, 2, 2)) {
    return INVALID_ENTITY;
  }

  EntityID entity = registry->CreateEntity();

  Vec2f worldPos = tileIndex * TILE_PIXEL_SIZE;

  registry->EmplaceComponent<TransformComponent>(entity,
                                                 TransformComponent{worldPos});

  BuildingComponent building;
  building.width = 2;
  building.height = 2;
  registry->EmplaceComponent<BuildingComponent>(entity, building);

  world->PlaceBuilding(entity, tileIndex, 2, 2);

  SDL_Texture *spritesheet =
      assetManager->getTexture("assets/img/entity/assembling-machine.png");

  SpriteComponent sprite;
  sprite.texture = spritesheet;

  sprite.srcRect = {0, 0, 214, 226};
  sprite.renderRect = {0, 10, 118, 124};
  registry->EmplaceComponent<SpriteComponent>(entity, sprite);

  // Add animation component
  AnimationComponent anim;

  // 32 sprites in 4 rows, 8 columns, starts idle (not playing)
  util::AddAnimation(anim, AnimationName::ASSEMBLING_MACHINE_IDLE, spritesheet,
                     {0, 1, 0.0f, 214, 226, false});
  util::AddAnimation(anim, AnimationName::ASSEMBLING_MACHINE_WORKING,
                     spritesheet, {0, 32, 8.0f, 214, 226, true});

  anim.currentAnimation = AnimationName::ASSEMBLING_MACHINE_IDLE;
  anim.isPlaying = false;
  registry->EmplaceComponent<AnimationComponent>(entity, anim);

  // Add assembling machine component
  AssemblingMachineComponent machine;
  registry->EmplaceComponent<AssemblingMachineComponent>(entity, machine);

  return entity;
}

EntityID EntityFactory::CreateMiningDrill(World *world, Vec2f worldPos) {
  if (registry == nullptr || world == nullptr) return INVALID_ENTITY;

  Vec2 tileIndex = world->GetTileIndexFromWorldPosition(worldPos);
  return CreateMiningDrill(world, tileIndex);
}

EntityID EntityFactory::CreateMiningDrill(World *world, Vec2 tileIndex) {
  if (registry == nullptr || world == nullptr) return INVALID_ENTITY;

  if (!world->CanPlaceBuilding(tileIndex, 1, 1)) {
    return INVALID_ENTITY;
  }

  EntityID entity = registry->CreateEntity();

  Vec2f centerPos = {
      static_cast<float>(tileIndex.x * TILE_PIXEL_SIZE),  // Center of 2x2 area
      static_cast<float>(tileIndex.y * TILE_PIXEL_SIZE)};

  Vec2f worldPos = tileIndex * TILE_PIXEL_SIZE;

  registry->EmplaceComponent<TransformComponent>(entity,
                                                 TransformComponent{worldPos});

  // Add building component
  BuildingComponent building;
  building.width = 1;
  building.height = 1;
  registry->EmplaceComponent<BuildingComponent>(entity, building);

  // Place the building in the world (this will update
  // BuildingComponent.occupiedTiles)
  world->PlaceBuilding(entity, tileIndex, 1, 1);

  // Add sprite component
  SDL_Texture *spritesheet =
      assetManager->getTexture("assets/img/entity/mining-drill.png");

  SpriteComponent sprite;
  sprite.texture = spritesheet;
  // Start with first frame (idle state)
  sprite.srcRect = {0, 0, 185, 168};
  sprite.renderRect = {0, 0, 62, 56};
  registry->EmplaceComponent<SpriteComponent>(entity, sprite);

  // Add animation component
  AnimationComponent anim;

  // 32 sprites in 4 rows, 8 columns, starts idle (not playing)
  util::AddAnimation(anim, AnimationName::DRILL_IDLE, spritesheet,
                     {0, 1, 0.0f, 185, 168, false});
  util::AddAnimation(anim, AnimationName::DRILL_WORKING, spritesheet,
                     {0, 32, 8.0f, 185, 168, true});

  anim.currentAnimation = AnimationName::DRILL_IDLE;
  anim.isPlaying = false;
  registry->EmplaceComponent<AnimationComponent>(entity, anim);

  MiningDrillComponent drill;

  if (TileData *tile = world->GetTileAtTileIndex(tileIndex)) {
    drill.oreEntity = tile->oreEntity;
  }
  registry->AddComponent<MiningDrillComponent>(entity, std::move(drill));
  registry->EmplaceComponent<InventoryComponent>(entity, InventoryComponent{});

  return entity;
}

EntityID EntityFactory::CreatePlayer(World *world, Vec2f worldPos) {
  if (registry == nullptr || world == nullptr) return INVALID_ENTITY;

  EntityID player = registry->CreateEntity();
  registry->EmplaceComponent<TransformComponent>(player, worldPos);

  SDL_Texture *playerIdleSpritesheet =
      assetManager->getTexture("assets/img/character/Miner_IdleAnimation.png");
  SDL_Texture *playerWalkSpritesheet =
      assetManager->getTexture("assets/img/character/Miner_WalkAnimation.png");
  SDL_Texture *playerMiningRightSpritesheet = assetManager->getTexture(
      "assets/img/character/Miner_MiningRightAnimation.png");
  SDL_Texture *playerMiningDownSpritesheet = assetManager->getTexture(
      "assets/img/character/Miner_MiningDownAnimation.png");

  registry->EmplaceComponent<SpriteComponent>(
      player, SpriteComponent{playerIdleSpritesheet,
                              {0, 0, 16, 16},
                              {-TILE_PIXEL_SIZE / 2, -TILE_PIXEL_SIZE / 2,
                               TILE_PIXEL_SIZE, TILE_PIXEL_SIZE},
                              SDL_FLIP_NONE,
                              render_order_t(100)});

  // Set Player Animation
  AnimationComponent anim;
  util::AddAnimation(anim, AnimationName::PLAYER_IDLE, playerIdleSpritesheet,
                     {0, 12, 10.f, 16, 16, true});
  util::AddAnimation(anim, AnimationName::PLAYER_WALK, playerWalkSpritesheet,
                     {0, 6, 15.f, 16, 16, true});
  util::AddAnimation(anim, AnimationName::PLAYER_MINE_RIGHT,
                     playerMiningRightSpritesheet, {0, 6, 15.f, 16, 16, true});
  util::AddAnimation(anim, AnimationName::PLAYER_MINE_DOWN,
                     playerMiningDownSpritesheet, {0, 6, 15.f, 16, 16, true});
  anim.currentAnimation = AnimationName::PLAYER_IDLE;

  registry->AddComponent<AnimationComponent>(player, std::move(anim));

  registry->EmplaceComponent<MovementComponent>(
      player, MovementComponent{.speed = 300.f});

  registry->EmplaceComponent<PlayerStateComponent>(
      player, PlayerStateComponent{.isMining = false,
                                   .interactingEntity = INVALID_ENTITY});

  registry->EmplaceComponent<MovableComponent>(player);

  registry->EmplaceComponent<InventoryComponent>(
      player, InventoryComponent{.row = 4, .column = 4});

  return player;
}

EntityFactory::~EntityFactory() = default;