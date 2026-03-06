#pragma once

#include <cstdint>

// Simple POD for local prediction/smoothing
struct NetPredictionComponent {
  // Predicted simulation position from local input (do not write to Transform
  // directly)
  float predictedX;
  float predictedY;

  // Smoothed visual offset applied on top of server base
  float offsetX;
  float offsetY;

  // Last input and idle timer (for dynamic delay/stop snap)
  uint8_t lastInputBit;
  float idleTime;

  // Optional: initialize-once flag
  uint8_t initialized;
};

#ifndef DISABLE_COMP_TYPECHECK
#include <type_traits>
static_assert(std::is_trivially_copyable_v<NetPredictionComponent> == true);
static_assert(std::is_trivially_destructible_v<NetPredictionComponent> == true);
#endif