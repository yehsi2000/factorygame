#include "System/InteractionSystem.h"

#include "Components/AnimationComponent.h"
#include "Components/AssemblingMachineComponent.h"
#include "Components/MiningDrillComponent.h"
#include "Components/PlayerStateComponent.h"
#include "Components/ResourceNodeComponent.h"
#include "Components/SpriteComponent.h"
#include "Components/TransformComponent.h"
#include "Core/Event.h"
#include "Core/EventDispatcher.h"
#include "Core/Registry.h"
#include "Core/World.h"
#include "Util/AnimUtil.h"
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
  Entity localPlayer = world->GetLocalPlayer();
  if (localPlayer == Entity::Null()) return;

  if (registry->HasComponent<PlayerStateComponent>(localPlayer)) {
    auto &playerStateComp =
        registry->GetComponent<PlayerStateComponent>(localPlayer);

    if (playerStateComp.isMining) {
      playerStateComp.isMining = false;

      auto &animComp = registry->GetComponent<AnimationComponent>(localPlayer);

      util::SetAnimation(AnimationName::PLAYER_IDLE, animComp, true);
      util::DetachTimer(registry, timerManager, localPlayer, TimerId::Mine);
    }
  }
}

void InteractionSystem::OnPlayerInteractEvent(
    const PlayerInteractEvent &event) {
  if (!registry || !world) return;

  Entity localPlayer = world->GetLocalPlayer();
  if (localPlayer == Entity::Null()) return;
  auto &ptrans = registry->GetComponent<TransformComponent>(localPlayer);
  if (maxInteractionDistance < util::dist(ptrans.position, event.target)) {
    return;
  }

  TileData *tile = world->GetTileAtWorldPosition(event.target);
  if (!tile) return;

  // Target occupying entity first
  Entity targetEntity = tile->occupyingEntity;

  // Target Ore if there's no entity
  if (targetEntity == Entity::Null() && tile->oreEntity != Entity::Null()) {
    targetEntity = tile->oreEntity;
  }

  if (!registry->HasComponent<PlayerStateComponent>(localPlayer)) return;

  if (registry->HasComponent<ResourceNodeComponent>(targetEntity)) {
    ResourceNodeInteractionHandler(localPlayer, targetEntity);
  } else if (registry->HasComponent<AssemblingMachineComponent>(targetEntity)) {
    AssemblyMachineInteractionHandler(localPlayer, targetEntity);
  } else if (registry->HasComponent<MiningDrillComponent>(targetEntity)) {
    MiningDrillInteractionHandler(localPlayer, targetEntity);
  }
}

void InteractionSystem::ResourceNodeInteractionHandler(Entity player,
                                                       Entity targetEntity) {
  if (!registry->HasComponent<PlayerStateComponent>(player) ||
      !registry->HasComponent<TransformComponent>(player) ||
      !registry->HasComponent<AnimationComponent>(player) ||
      !registry->HasComponent<SpriteComponent>(player) ||
      !registry->HasComponent<TransformComponent>(targetEntity)) {
    return;
  }

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

void InteractionSystem::AssemblyMachineInteractionHandler(Entity player,
                                                          Entity targetEntity) {
  auto &machine =
      registry->GetComponent<AssemblingMachineComponent>(targetEntity);

  machine.isShowingUI = true;
  // If no recipe is selected, show recipe selection
  if (machine.currentRecipe == RecipeID::None) {
    machine.isRecipeSelected = false;
  } else {
    machine.isRecipeSelected = true;
  }
}

void InteractionSystem::MiningDrillInteractionHandler(Entity player,
                                                      Entity targetEntity) {
  auto &drill = registry->GetComponent<MiningDrillComponent>(targetEntity);
  drill.isShowingUI = true;
}

void InteractionSystem::Update() {}

InteractionSystem::~InteractionSystem() = default;