#pragma once
#include <cstdint>

struct InputStateComponent {
  uint8_t inputBit;   // RIGHT/LEFT/UP/DOWN bits
  uint16_t sequence;  // Last processed input sequence from client
};

#ifndef DISABLE_COMP_TYPECHECK
#include <type_traits>
static_assert(std::is_trivially_copyable_v<InputStateComponent> == true);
static_assert(std::is_trivially_destructible_v<InputStateComponent> == true);
#endif