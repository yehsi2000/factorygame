#include "Util/EntityFactory.h"
#include "Components/AssemblingMachineComponent.h"
#include "Components/AnimationComponent.h"
#include "Components/BuildingComponent.h"
#include "Components/SpriteComponent.h"
#include "Components/TransformComponent.h"
#include "Core/AssetManager.h"
#include "Core/Registry.h"
#include "Core/TileData.h"
#include "Core/World.h"
#include "SDL.h"

namespace factory {

EntityID CreateAssemblingMachine(Registry& registry, World& world, 
                                 SDL_Renderer* renderer, Vec2f worldPos) {
  Vec2 tileIndex = world.GetTileIndexFromWorldPosition(worldPos);
  return CreateAssemblingMachine(registry, world, renderer, tileIndex);
}

EntityID CreateAssemblingMachine(Registry& registry, World& world, 
                                 SDL_Renderer* renderer, Vec2 tileIndex) {

  if (!world.CanPlaceBuilding(tileIndex, 2, 2)) {
    return INVALID_ENTITY;
  }
  
  EntityID entity = registry.CreateEntity();
  
  Vec2f centerPos = {
    static_cast<float>(tileIndex.x * TILE_PIXEL_SIZE + TILE_PIXEL_SIZE),  // Center of 2x2 area
    static_cast<float>(tileIndex.y * TILE_PIXEL_SIZE + TILE_PIXEL_SIZE)
  };

  Vec2f worldPos = tileIndex * TILE_PIXEL_SIZE;

  registry.EmplaceComponent<TransformComponent>(entity, 
    TransformComponent{worldPos});
  
  // Add building component
  BuildingComponent building;
  building.width = 2;
  building.height = 2;
  registry.EmplaceComponent<BuildingComponent>(entity, building);
  
  // Place the building in the world (this will update BuildingComponent.occupiedTiles)
  world.PlaceBuilding(entity, tileIndex, 2, 2);
  
  // Add sprite component
  SDL_Texture* spritesheet = AssetManager::Instance().getTexture(
      "assets/img/entity/assembling-machine.png", renderer);
  
  SpriteComponent sprite;
  sprite.texture = spritesheet;
  // Start with first frame (idle state)
  sprite.srcRect = {0, 0, 214, 226};  // Single sprite size from 1712x904 spritesheet
  sprite.renderRect = {53, 56, 107, 113}; // Scale to 0.5 (214*0.5 = 107, 226*0.5 = 113)
  registry.EmplaceComponent<SpriteComponent>(entity, sprite);
  
  // Add animation component
  AnimationComponent anim;
  // 32 sprites in 4 rows, 8 columns, starts idle (not playing)
  anim.animations["idle"] = {0, 1, 0.0f, 214, 226, false}; // Single idle frame
  anim.animations["working"] = {0, 32, 8.0f, 214, 226, true}; // All 32 frames looping
  anim.currentAnimationName = "idle";
  anim.isPlaying = false;
  registry.EmplaceComponent<AnimationComponent>(entity, anim);
  
  // Add assembling machine component
  AssemblingMachineComponent machine;
  registry.EmplaceComponent<AssemblingMachineComponent>(entity, machine);
  
  return entity;
}

} // namespace util