#ifndef COMPONENTS_PLAYERSTATECOMPONENT_
#define COMPONENTS_PLAYERSTATECOMPONENT_

#include <cstdint>

#include "Core/Entity.h"
#include "Core/Packet.h"


struct PlayerStateComponent {
  bool bIsMining;
  EntityID interactingEntity;
  clientid_t clientID;
  uint16_t lastProcessedSeq;
  uint16_t lastSeqSentToClient;  // last acked seq sent to client
};

#endif /* COMPONENTS_PLAYERSTATECOMPONENT_ */
