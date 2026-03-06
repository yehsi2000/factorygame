#pragma once

struct MovementComponent {
  float speed;  // per pixel
};

#ifndef DISABLE_COMP_TYPECHECK
#include <type_traits>
static_assert(std::is_trivially_copyable_v<MovementComponent> == true);
static_assert(std::is_trivially_destructible_v<MovementComponent> == true);
#endif