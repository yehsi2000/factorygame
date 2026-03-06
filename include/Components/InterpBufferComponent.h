#pragma once
#include <cstdint>
#include "DataStruct/RingBuffer.h"

struct InterpBufferComponent {
  struct Sample {
    double time;  // seconds (steady clock)
    float x, y;
    uint8_t facing;
  };
  // static constexpr uint8_t N = 16;
  // uint8_t tail;   // index of oldest
  // uint8_t count;  // number of valid samples
  // Sample samples[N];
  RingBuffer<Sample, 16> samples;
};

#ifndef DISABLE_COMP_TYPECHECK
#include <type_traits>
static_assert(std::is_trivially_copyable_v<InterpBufferComponent> == true);
static_assert(std::is_trivially_destructible_v<InterpBufferComponent> == true);
#endif