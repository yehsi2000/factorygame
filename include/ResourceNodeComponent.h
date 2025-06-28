#pragma once
#include <memory>

#include "Component.h"
#include "Entity.h"
#include "Item.h"

struct ResourceNodeComponent : public Component {
  ResourceNodeComponent(int totalResource, OreType oreType)
      : LeftResource(totalResource), Miner(0), Ore(oreType) {};
  long long LeftResource;
  EntityID Miner;
  OreType Ore;
};
