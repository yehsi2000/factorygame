#pragma once

#include "Core/Entity.h"
#include "Core/Packet.h"

struct PlayerStateComponent {
  bool isMining;
  Entity interactingEntity;
  clientid_t clientID;
};

#ifndef DISABLE_COMP_TYPECHECK
#include <type_traits>
static_assert(std::is_trivially_copyable_v<PlayerStateComponent> == true);
static_assert(std::is_trivially_destructible_v<PlayerStateComponent> == true);
#endif