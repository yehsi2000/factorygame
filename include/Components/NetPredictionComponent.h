#ifndef COMPONENTS_NETPREDICTIONCOMPONENT_
#define COMPONENTS_NETPREDICTIONCOMPONENT_

#include <cstdint>

struct NetPredictionComponent {
  uint16_t lastSentSeq, lastAckSeq;
  uint8_t pendingCnt;
  struct Pending {
    uint16_t seq;
    float nx, ny;
    float dt;
  } pending[32];
};

#endif /* COMPONENTS_NETPREDICTIONCOMPONENT_ */
