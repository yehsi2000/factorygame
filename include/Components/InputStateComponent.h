#pragma once
#include <cstdint>
struct InputStateComponent {
  uint8_t inputBit = 0;  // RIGHT/LEFT/UP/DOWN bits
  uint16_t sequence = 0; // Last processed input sequence from client
};