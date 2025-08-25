#include "System/InteractionSystem.h"

#include "Components/AnimationComponent.h"
#include "Components/AssemblingMachineComponent.h"
#include "Components/MiningDrillComponent.h"
#include "Components/PlayerStateComponent.h"
#include "Components/ResourceNodeComponent.h"
#include "Components/SpriteComponent.h"
#include "Components/TransformComponent.h"

#include "Core/GEngine.h"
#include "Core/Registry.h"
#include "Core/World.h"

#include "Util/AnimUtil.h"
#include "Util/MathUtil.h"
#include "Util/TimerUtil.h"

InteractionSystem::InteractionSystem(GEngine *engine)
    : engine(engine),
      handle(engine->GetDispatcher()->Subscribe<PlayerInteractEvent>(
          [this](const PlayerInteractEvent &event) {
            this->OnPlayerInteractEvent(event);
          })) {}

void InteractionSystem::OnPlayerInteractEvent(
    const PlayerInteractEvent &event) {
  Registry *reg = engine->GetRegistry();
  World *world = engine->GetWorld();

  if (!reg || !world)
    return;

  EntityID player = engine->GetPlayer();

  TileData *tile = world->GetTileAtTileIndex(event.target);
  if (!tile)
    return;

  // Target occupying entity first
  EntityID targetEntity = tile->occupyingEntity;

  // Target Ore if there's no entity
  if (targetEntity == INVALID_ENTITY && tile->oreEntity != INVALID_ENTITY) {
    targetEntity = tile->oreEntity;
  }

  if (!reg->HasComponent<PlayerStateComponent>(player))
    return;

  if (reg->HasComponent<ResourceNodeComponent>(targetEntity)) {
    auto &playerStateComp = reg->GetComponent<PlayerStateComponent>(player);
    auto &playerTransComp = reg->GetComponent<TransformComponent>(player);
    auto &playerAnimComp = reg->GetComponent<AnimationComponent>(player);
    auto &playerSpriteComp = reg->GetComponent<SpriteComponent>(player);
    auto &oreTransComp = reg->GetComponent<TransformComponent>(targetEntity);

    playerStateComp.isMining = true;
    playerStateComp.interactingEntity = targetEntity;

    Vec2f dir = (oreTransComp.position - playerTransComp.position);

    // Set mining animation
    if (util::dist(dir) < 10.f || std::abs(dir.x) < std::abs(dir.y)) {
      // Mine Down
      util::SetAnimation(AnimationName::PLAYER_MINE_DOWN, playerAnimComp, true);
      playerSpriteComp.flip = SDL_FLIP_NONE;
    } else {
      // Mine Side
      util::SetAnimation(AnimationName::PLAYER_MINE_RIGHT, playerAnimComp,
                         true);
      if (dir.x >= 0) {
        playerSpriteComp.flip = SDL_FLIP_NONE;
      } else {
        playerSpriteComp.flip = SDL_FLIP_HORIZONTAL;
      }
    }

    util::AttachTimer(reg, engine->GetTimerManager(), player, TimerId::Mine,
                      1.0f, true);

  } else if (reg->HasComponent<AssemblingMachineComponent>(targetEntity)) {
    auto &machine = reg->GetComponent<AssemblingMachineComponent>(targetEntity);

    machine.showUI = true;
    // If no recipe is selected, show recipe selection
    if (machine.currentRecipe == RecipeID::None) {
      machine.showRecipeSelection = true;
    } else {
      machine.showRecipeSelection = false;
    }

  } else if (reg->HasComponent<MiningDrillComponent>(targetEntity)) {
    auto &drill = reg->GetComponent<MiningDrillComponent>(targetEntity);
    drill.showUI = true;
  }
}

void InteractionSystem::Update() {}