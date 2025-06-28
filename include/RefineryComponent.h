#pragma once

#include "Component.h"
#include "Entity.h"

struct RefineryComponent : public Component {
  RefineryComponent() : connectedMiner(0) {};
  EntityID connectedMiner;
};
