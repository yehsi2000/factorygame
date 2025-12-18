#pragma once
 
#include "Core/Item.h"

using rsrc_amt_t = unsigned int;

struct ResourceNodeComponent {
  rsrc_amt_t LeftResource;
  OreType Ore;
};
