#ifndef COMPONENTS_MOVEINTENTCOMPONENT_
#define COMPONENTS_MOVEINTENTCOMPONENT_

#include <cstdint>
#include "Core/Packet.h"

struct MoveIntentComponent {
  uint8_t tail = 0;   // index of oldest entry
  uint8_t count = 0;  // number of valid entries [0..32]
  uint16_t lastEnqueuedSeq =
      0;  // used by host (server-local player) to generate seqs

  struct Pending {
    uint16_t seq;
    uint8_t inputBit;
    float dt;
  } pending[NET_BUFFER_SIZE];
};

#endif /* COMPONENTS_MOVEINTENTCOMPONENT_ */
