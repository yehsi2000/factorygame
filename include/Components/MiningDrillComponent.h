#ifndef COMPONENTS_MININGDRILLCOMPONENT_
#define COMPONENTS_MININGDRILLCOMPONENT_

#include <utility>

#include "Core/Entity.h"
#include "Core/Item.h"

enum class MiningDrillState { Idle, TileEmpty, Mining, OutputFull };

struct MiningDrillComponent {
  MiningDrillState state;
  bool bIsAnimating;
  bool bIsShowingUI;
  EntityID oreEntity;

  constexpr MiningDrillComponent(
      MiningDrillState state = MiningDrillState::Idle, bool bIsAnimating = false,
      bool bIsShowingUI = false,
      std::pair<ItemID, int> outputSlot = {ItemID::None, 0})
      : state(state),
        bIsAnimating(bIsAnimating),
        bIsShowingUI(bIsShowingUI),
        oreEntity(INVALID_ENTITY) {}
};

#endif /* COMPONENTS_MININGDRILLCOMPONENT_ */
