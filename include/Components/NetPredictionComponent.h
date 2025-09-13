#ifndef COMPONENTS_NETPREDICTIONCOMPONENT_
#define COMPONENTS_NETPREDICTIONCOMPONENT_

#include <cstdint>
#include "Core/Packet.h"

// Simple POD for local prediction/smoothing
struct NetPredictionComponent {
  // Predicted simulation position from local input (do not write to Transform directly)
  float predictedX = 0.f;
  float predictedY = 0.f;

  // Smoothed visual offset applied on top of server base
  float offsetX = 0.f;
  float offsetY = 0.f;

  // Last input and idle timer (for dynamic delay/stop snap)
  uint8_t lastInputBit = 0;
  float idleTime = 0.f;

  // Optional: initialize-once flag
  uint8_t initialized = 0;
};

#endif /* COMPONENTS_NETPREDICTIONCOMPONENT_ */
