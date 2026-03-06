#pragma once

/**
 * @brief // Marks components inactive to ignore entities that currently does
 * nothing.
 */
struct InactiveComponent {
  bool isInactive;
};

#ifndef DISABLE_COMP_TYPECHECK
#include <type_traits>
static_assert(std::is_trivially_copyable_v<InactiveComponent> == true);
static_assert(std::is_trivially_destructible_v<InactiveComponent> == true);
#endif