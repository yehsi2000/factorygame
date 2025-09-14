#ifndef COMPONENTS_PLAYERSTATECOMPONENT_
#define COMPONENTS_PLAYERSTATECOMPONENT_

#include <cstdint>

#include "Core/Entity.h"
#include "Core/Packet.h"


struct PlayerStateComponent {
  bool bIsMining;
  EntityID interactingEntity;
  clientid_t clientID;
};

#endif /* COMPONENTS_PLAYERSTATECOMPONENT_ */
