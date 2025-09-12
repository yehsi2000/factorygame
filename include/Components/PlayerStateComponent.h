#ifndef COMPONENTS_PLAYERSTATECOMPONENT_
#define COMPONENTS_PLAYERSTATECOMPONENT_

#include "Core/Entity.h"

struct PlayerStateComponent{
  bool bIsMining;
  EntityID interactingEntity;
};

#endif/* COMPONENTS_PLAYERSTATECOMPONENT_ */
