#pragma once

#include "Core/Item.h"

using rsrc_amt_t = unsigned int;

struct ResourceNodeComponent {
  rsrc_amt_t LeftResource;
  OreType Ore;
};

#ifndef DISABLE_COMP_TYPECHECK
#include <type_traits>
static_assert(std::is_trivially_copyable_v<ResourceNodeComponent> == true);
static_assert(std::is_trivially_destructible_v<ResourceNodeComponent> == true);
#endif