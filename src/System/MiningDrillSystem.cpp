#include "System/MiningDrillSystem.h"

#include "Components/AnimationComponent.h"
#include "Components/InventoryComponent.h"
#include "Components/MiningDrillComponent.h"
#include "Components/ResourceNodeComponent.h"
#include "Components/TransformComponent.h"
#include "Core/Entity.h"
#include "Core/Item.h"
#include "Core/Registry.h"
#include "Core/TimerManager.h"
#include "Core/World.h"
#include "Util/AnimUtil.h"
#include "Util/TimerUtil.h"

MiningDrillSystem::MiningDrillSystem(const SystemContext& context)
    : registry(context.registry),
      world(context.world),
      timerManager(context.timerManager) {}

void MiningDrillSystem::Update() {
  for (auto& entity : registry->view<MiningDrillComponent, InventoryComponent,
                                     TransformComponent>()) {
    auto& drill = registry->GetComponent<MiningDrillComponent>(entity);
    auto& inv = registry->GetComponent<InventoryComponent>(entity);

    switch (drill.state) {
      // Initial State
      case MiningDrillState::Idle: {
        if (TileEmpty(entity)) {
          drill.state = MiningDrillState::TileEmpty;
        } else {
          StartMining(drill, entity);
        }
        break;
      }

      case MiningDrillState::Mining: {
        if (TileEmpty(entity)) {
          drill.state = MiningDrillState::TileEmpty;
        } else if (inv.items.size() > 0 &&
                   ItemDatabase::instance()
                           .get(inv.items[0].first)
                           .maxStackSize <= inv.items[0].second) {
          drill.state = MiningDrillState::OutputFull;
        } else
          continue;  // continue mining

        util::DetachTimer(registry, timerManager, entity, TimerId::Mine);
        drill.bIsAnimating = false;
        break;
      }

      case MiningDrillState::TileEmpty: {
        continue;
      }

      case MiningDrillState::OutputFull: {
        if (inv.items.size() == 0) {
          StartMining(drill, entity);
          break;
        }

        int maxStackSize =
            ItemDatabase::instance().get(inv.items[0].first).maxStackSize;

        if (maxStackSize > inv.items[0].second) {
          drill.state = MiningDrillState::Mining;
          StartMining(drill, entity);
        }

        break;
      }
    }

    UpdateAnimationState(drill, entity);
  }
}

void MiningDrillSystem::StartMining(MiningDrillComponent& drill,
                                    EntityID entity) {
  auto& transform = registry->GetComponent<TransformComponent>(entity);

  TileData* tile = world->GetTileAtWorldPosition(transform.position);

  if (!tile || tile->oreEntity == INVALID_ENTITY) return;

  if (registry->HasComponent<ResourceNodeComponent>(tile->oreEntity)) {
    auto& resnode =
        registry->GetComponent<ResourceNodeComponent>(tile->oreEntity);

    if (resnode.LeftResource == 0) {
      drill.state = MiningDrillState::TileEmpty;
      return;
    } else {
      drill.state = MiningDrillState::Mining;
      drill.bIsAnimating = true;
      util::AttachTimer(registry, timerManager, entity, TimerId::Mine, 1.f,
                        true);
    }
  }
}

bool MiningDrillSystem::TileEmpty(EntityID entity) {
  auto& transform = registry->GetComponent<TransformComponent>(entity);

  if (TileData* tile = world->GetTileAtWorldPosition(transform.position)) {
    if (tile->oreEntity != INVALID_ENTITY &&
        registry->HasComponent<ResourceNodeComponent>(tile->oreEntity)) {
      auto& resNode =
          registry->GetComponent<ResourceNodeComponent>(tile->oreEntity);
      if (resNode.LeftResource > 0) return false;
    }
  }
  return true;
}

void MiningDrillSystem::UpdateAnimationState(MiningDrillComponent& drill,
                                             EntityID entity) {
  if (!registry->HasComponent<AnimationComponent>(entity)) return;

  auto& animation = registry->GetComponent<AnimationComponent>(entity);

  if (drill.bIsAnimating &&
      animation.currentAnimation != AnimationName::DRILL_WORKING) {
    util::SetAnimation(AnimationName::DRILL_WORKING, animation, true);
  } else if (!drill.bIsAnimating &&
             animation.currentAnimation != AnimationName::DRILL_IDLE) {
    util::SetAnimation(AnimationName::DRILL_IDLE, animation, true);
  } else {
    animation.bIsPlaying = drill.bIsAnimating;
  }
}

MiningDrillSystem::~MiningDrillSystem() = default;