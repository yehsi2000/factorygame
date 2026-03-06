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

#ifndef DISABLE_COMP_TYPECHECK
#include <type_traits>
static_assert(std::is_trivially_copyable_v<MiningDrillComponent> == true);
static_assert(std::is_trivially_destructible_v<MiningDrillComponent> == true);
#endif