#ifndef COMPONENTS_REFINERYCOMPONENT_
#define COMPONENTS_REFINERYCOMPONENT_

#include "Core/Entity.h"

struct RefineryComponent {
  Entity connectedMiner = Entity::Null();
};

#endif /* COMPONENTS_REFINERYCOMPONENT_ */
