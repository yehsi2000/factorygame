#pragma once

struct DebugRectComponent {
  int offsetX, offsetY;
  int width, height;
  int r, g, b, a;
};

#ifndef DISABLE_COMP_TYPECHECK
#include <type_traits>
static_assert(std::is_trivially_copyable_v<DebugRectComponent> == true);
static_assert(std::is_trivially_destructible_v<DebugRectComponent> == true);
#endif
