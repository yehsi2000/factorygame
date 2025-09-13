#pragma once
#include <cstdint>

struct InterpBufferComponent {
  struct Sample {
    double t;     // seconds (steady clock)
    float x, y;
    uint8_t facing;
  };
  static constexpr uint8_t N = 16;
  uint8_t tail = 0;   // index of oldest
  uint8_t count = 0;  // number of valid samples
  Sample samples[N];
};