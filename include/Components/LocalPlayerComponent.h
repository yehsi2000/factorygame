#pragma once

#include "Core/Packet.h"

struct LocalPlayerComponent {
  clientid_t clientID;
};

#ifndef DISABLE_COMP_TYPECHECK
#include <type_traits>
static_assert(std::is_trivially_copyable_v<LocalPlayerComponent> == true);
static_assert(std::is_trivially_destructible_v<LocalPlayerComponent> == true);
#endif
