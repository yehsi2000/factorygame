#ifndef COMPONENTS_MININGDRILLCOMPONENT_
#define COMPONENTS_MININGDRILLCOMPONENT_

#include <utility>

#include "Core/Entity.h"
#include "Core/Item.h"

enum class MiningDrillState { Idle, TileEmpty, Mining, OutputFull };

struct MiningDrillComponent {
  MiningDrillState state;
  bool isAnimating;
  bool showUI;
  EntityID oreEntity;

  constexpr MiningDrillComponent(
      MiningDrillState state = MiningDrillState::Idle, bool isAnimating = false,
      bool showUI = false,
      std::pair<ItemID, int> outputSlot = {ItemID::None, 0})
      : state(state),
        isAnimating(isAnimating),
        showUI(showUI),
        oreEntity(INVALID_ENTITY) {}
};

#endif /* COMPONENTS_MININGDRILLCOMPONENT_ */
