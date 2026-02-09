#pragma once
#include <cstdint>

struct InputStateComponent {
  uint8_t inputBit;   // RIGHT/LEFT/UP/DOWN bits
  uint16_t sequence;  // Last processed input sequence from client
};