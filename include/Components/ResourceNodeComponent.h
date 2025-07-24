#ifndef COMPONENTS_RESOURCENODECOMPONENT_
#define COMPONENTS_RESOURCENODECOMPONENT_

#include <memory>

#include "Entity.h"
#include "Item.h"

struct ResourceNodeComponent {
  long long LeftResource;
  OreType Ore;
};

#endif /* COMPONENTS_RESOURCENODECOMPONENT_ */
