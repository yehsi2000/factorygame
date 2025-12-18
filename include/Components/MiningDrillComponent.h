#pragma once
 
#include <utility>

#include "Core/Entity.h"
#include "Core/Item.h"

enum class MiningDrillState { Idle=0, TileEmpty, Mining, OutputFull };

struct MiningDrillComponent {
  MiningDrillState state;
  bool bIsAnimating;
  bool bIsShowingUI;
  Entity oreEntity;

  constexpr MiningDrillComponent() = default;

  // constexpr MiningDrillComponent(
  //     MiningDrillState state = MiningDrillState::Idle, bool bIsAnimating = false,
  //     bool bIsShowingUI = false,
  //     std::pair<ItemID, int> outputSlot = {ItemID::None, 0})
  //     : state(state),
  //       bIsAnimating(bIsAnimating),
  //       bIsShowingUI(bIsShowingUI),
  //       oreEntity(Entity::Null()) {}
};
