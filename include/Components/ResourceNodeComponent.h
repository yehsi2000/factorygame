#ifndef COMPONENTS_RESOURCENODECOMPONENT_
#define COMPONENTS_RESOURCENODECOMPONENT_

#include <memory>

#include "Entity.h"
#include "Item.h"

using rsrc_amt_t = unsigned long long;

struct ResourceNodeComponent {
  rsrc_amt_t LeftResource;
  OreType Ore;
};

#endif /* COMPONENTS_RESOURCENODECOMPONENT_ */
