#pragma once

struct MovableComponent {};

#ifndef DISABLE_COMP_TYPECHECK
#include <type_traits>
static_assert(std::is_trivially_copyable_v<MovableComponent> == true);
static_assert(std::is_trivially_destructible_v<MovableComponent> == true);
#endif