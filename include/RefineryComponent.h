#include "Entity.h"

struct RefineryComponent {
  RefineryComponent() : connectedMiner(0) {};
  EntityID connectedMiner;
};
