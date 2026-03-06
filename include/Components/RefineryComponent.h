#pragma once

struct RefineryComponent {};

#ifndef DISABLE_COMP_TYPECHECK
#include <type_traits>
static_assert(std::is_trivially_copyable_v<RefineryComponent> == true);
static_assert(std::is_trivially_destructible_v<RefineryComponent> == true);
#endif