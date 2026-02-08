#pragma once
 

#include "Core/Entity.h"

enum class MiningDrillState { Idle=0, TileEmpty, Mining, OutputFull };

struct MiningDrillComponent {
  MiningDrillState state;
  bool isAnimating;
  bool isShowingUI;
  Entity oreEntity;

  constexpr MiningDrillComponent() = default;

  // constexpr MiningDrillComponent(
  //     MiningDrillState state = MiningDrillState::Idle, bool isAnimating = false,
  //     bool isShowingUI = false,
  //     std::pair<ItemID, int> outputSlot = {ItemID::None, 0})
  //     : state(state),
  //       isAnimating(isAnimating),
  //       isShowingUI(isShowingUI),
  //       oreEntity(Entity::Null()) {}
};
