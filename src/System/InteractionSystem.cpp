#include "System/InteractionSystem.h"

#include <optional>

#include "Components/AnimationComponent.h"
#include "Components/AssemblingMachineComponent.h"
#include "Components/MiningDrillComponent.h"
#include "Components/PlayerStateComponent.h"
#include "Components/ResourceNodeComponent.h"
#include "Components/SpriteComponent.h"
#include "Components/TransformComponent.h"
#include "Core/Event.h"
#include "Core/InputManager.h"
#include "Core/EventDispatcher.h"
#include "Core/Registry.h"
#include "Core/TimerManager.h"
#include "Core/World.h"
#include "Util/AnimUtil.h"
#include "Util/CameraUtil.h"
#include "Util/MathUtil.h"
#include "Util/TimerUtil.h"

InteractionSystem::InteractionSystem(const SystemContext &context)
    : registry(context.registry),
      world(context.world),
      eventDispatcher(context.eventDispatcher),
      inputManager(context.inputManager),
      timerManager(context.timerManager) {
  startInteractHandle = eventDispatcher->Subscribe<PlayerInteractEvent>(
      [this](const PlayerInteractEvent &event) {
        this->OnPlayerInteractEvent(event);
      });
  endInteractHandle = eventDispatcher->Subscribe<PlayerEndInteractEvent>(
      [this](const PlayerEndInteractEvent &event) {
        this->OnPlayerEndInteractEvent(event);
      });
}

void InteractionSystem::OnPlayerEndInteractEvent(
    const PlayerEndInteractEvent &event) {
  EntityID player = world->GetPlayer();

  if (registry->HasComponent<PlayerStateComponent>(player)) {
    auto &playerStateComp =
        registry->GetComponent<PlayerStateComponent>(player);

    if (playerStateComp.isMining) {
      playerStateComp.isMining = false;

      auto &animComp = registry->GetComponent<AnimationComponent>(player);

      util::SetAnimation(AnimationName::PLAYER_IDLE, animComp, true);
      util::DetachTimer(registry, timerManager, player, TimerId::Mine);
    }
  }
}

void InteractionSystem::OnPlayerInteractEvent(
    const PlayerInteractEvent &event) {
  if (!registry || !world) return;

  EntityID player = world->GetPlayer();
  auto& ptrans = registry->GetComponent<TransformComponent>(player);
  if(maxInteractionDistance < util::dist(ptrans.position, event.target)){
    return;
  }

  TileData *tile = world->GetTileAtWorldPosition(event.target);
  if (!tile) return;

  // Target occupying entity first
  EntityID targetEntity = tile->occupyingEntity;

  // Target Ore if there's no entity
  if (targetEntity == INVALID_ENTITY && tile->oreEntity != INVALID_ENTITY) {
    targetEntity = tile->oreEntity;
  }

  if (!registry->HasComponent<PlayerStateComponent>(player)) return;

  if (registry->HasComponent<ResourceNodeComponent>(targetEntity)) {
    ResourceNodeInteractionHandler(player, targetEntity);
  } else if (registry->HasComponent<AssemblingMachineComponent>(targetEntity)) {
    AssemblyMachineInteractionHandler(player, targetEntity);
  } else if (registry->HasComponent<MiningDrillComponent>(targetEntity)) {
    MiningDrillInteractionHandler(player, targetEntity);
  }
}

void InteractionSystem::ResourceNodeInteractionHandler(EntityID player,
                                                       EntityID targetEntity) {
  auto &playerStateComp = registry->GetComponent<PlayerStateComponent>(player);
  auto &playerTransComp = registry->GetComponent<TransformComponent>(player);
  auto &playerAnimComp = registry->GetComponent<AnimationComponent>(player);
  auto &playerSpriteComp = registry->GetComponent<SpriteComponent>(player);
  auto &oreTransComp = registry->GetComponent<TransformComponent>(targetEntity);

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
    util::SetAnimation(AnimationName::PLAYER_MINE_RIGHT, playerAnimComp, true);
    if (dir.x >= 0) {
      playerSpriteComp.flip = SDL_FLIP_NONE;
    } else {
      playerSpriteComp.flip = SDL_FLIP_HORIZONTAL;
    }
  }

  util::AttachTimer(registry, timerManager, player, TimerId::Mine, 1.0f, true);
}

void InteractionSystem::AssemblyMachineInteractionHandler(
    EntityID player, EntityID targetEntity) {
  auto &machine =
      registry->GetComponent<AssemblingMachineComponent>(targetEntity);

  machine.showUI = true;
  // If no recipe is selected, show recipe selection
  if (machine.currentRecipe == RecipeID::None) {
    machine.showRecipeSelection = true;
  } else {
    machine.showRecipeSelection = false;
  }
}

void InteractionSystem::MiningDrillInteractionHandler(EntityID player,
                                                      EntityID targetEntity) {
  auto &drill = registry->GetComponent<MiningDrillComponent>(targetEntity);
  drill.showUI = true;
}

void InteractionSystem::Update() {}

InteractionSystem::~InteractionSystem() = default;