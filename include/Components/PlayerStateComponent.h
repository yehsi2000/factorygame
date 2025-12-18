#pragma once
 
#include <cstdint>

#include "Core/Entity.h"
#include "Core/Packet.h"


struct PlayerStateComponent {
  bool bIsMining;
  Entity interactingEntity;
  clientid_t clientID;
};
