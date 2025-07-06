#include <memory>

#include "Entity.h"

enum class OreType;

struct ResourceNodeComponent {
  ResourceNodeComponent(int totalResource, OreType oreType)
      : LeftResource(totalResource), Miner(0), Ore(oreType) {};
  long long LeftResource;
  EntityID Miner;
  OreType Ore;
};
