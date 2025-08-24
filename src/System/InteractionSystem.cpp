#include "System/InteractionSystem.h"

#include <Components/MiningDrillComponent.h>
#include <Components/PlayerStateComponent.h>

#include "Components/AssemblingMachineComponent.h"
#include "Components/ResourceNodeComponent.h"
#include "Core/GEngine.h"
#include "Core/Registry.h"
#include "Core/World.h"
#include "Util/TimerUtil.h"

InteractionSystem::InteractionSystem(GEngine* engine)
    : engine(engine),
      handle(engine->GetDispatcher()->Subscribe<PlayerInteractEvent>(
          [this](const PlayerInteractEvent& event) {
            this->OnPlayerInteractEvent(event);
          })) {}

void InteractionSystem::OnPlayerInteractEvent(const PlayerInteractEvent& event) {
  Registry* reg = engine->GetRegistry();
  World* world = engine->GetWorld();

  if (!reg || !world) return;

  EntityID player = engine->GetPlayer();

  TileData* tile = world->GetTileAtTileIndex(event.target);
  if (!tile) return;

  // Target occupying entity first
  EntityID targetEntity = tile->occupyingEntity;

  // Target Ore if there's no entity
  if (targetEntity == INVALID_ENTITY && tile->oreEntity != INVALID_ENTITY) {
    targetEntity = tile->oreEntity;
  }

  if (!reg->HasComponent<PlayerStateComponent>(player)) return;

  if (reg->HasComponent<ResourceNodeComponent>(targetEntity)) {  
    auto& state = reg->GetComponent<PlayerStateComponent>(player);
    state.isMining = true;
    state.interactingEntity = targetEntity;

    util::AttachTimer(reg, engine->GetTimerManager(), player,
                      TimerId::Mine, 1.0f, true);

  } else if (reg->HasComponent<AssemblingMachineComponent>(targetEntity)) {
    auto& machine =
        reg->GetComponent<AssemblingMachineComponent>(targetEntity);

    machine.showUI = true;
    // If no recipe is selected, show recipe selection
    if (machine.currentRecipe == RecipeID::None) {
      machine.showRecipeSelection = true;
    } else {
      machine.showRecipeSelection = false;
    }

  } else if (reg->HasComponent<MiningDrillComponent>(targetEntity)) {
    auto& drill = reg->GetComponent<MiningDrillComponent>(targetEntity);
    drill.showUI = true;
  }
}

void InteractionSystem::Update() {}