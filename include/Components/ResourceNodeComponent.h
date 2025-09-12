#ifndef COMPONENTS_RESOURCENODECOMPONENT_
#define COMPONENTS_RESOURCENODECOMPONENT_

#include "Core/Item.h"

using rsrc_amt_t = unsigned int;

struct ResourceNodeComponent {
  rsrc_amt_t LeftResource;
  OreType Ore;
};

#endif /* COMPONENTS_RESOURCENODECOMPONENT_ */
