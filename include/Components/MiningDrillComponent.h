#pragma once

#include "Core/Entity.h"

enum class MiningDrillState { Idle = 0, TileEmpty, Mining, OutputFull };

struct MiningDrillComponent {
  MiningDrillState state;
  bool isAnimating;
  bool isShowingUI;
  Entity oreEntity;

  constexpr MiningDrillComponent() = default;
};
