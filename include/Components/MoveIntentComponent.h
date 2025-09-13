#ifndef COMPONENTS_MOVEINTENTCOMPONENT_
#define COMPONENTS_MOVEINTENTCOMPONENT_

#include <cstdint>

struct MoveIntentComponent {
  uint16_t seq;
  uint8_t inputBit;
  float deltaTime;
  bool hasNew;
};

#endif/* COMPONENTS_MOVEINTENTCOMPONENT_ */
