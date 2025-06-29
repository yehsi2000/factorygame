#pragma once
#include <memory>

#include "Entity.h"
#include "Item.h"

struct ResourceNodeComponent {
  ResourceNodeComponent(int totalResource, OreType oreType)
      : LeftResource(totalResource), Miner(0), Ore(oreType) {};
  long long LeftResource;
  EntityID Miner;
  OreType Ore;
};
