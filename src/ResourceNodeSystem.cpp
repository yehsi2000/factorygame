#include "ResourceNodeSystem.h"

#include "ResourceNodeComponent.h"
#include "World.h"

void ResourceNodeSystem::Update(World& world) {
}

void ResourceNodeSystem::AddMiner(ResourceNodeComponent& resNode,
                                  EntityID player) {
  resNode.Miner = player;
}

void ResourceNodeSystem::RemoveMiner(ResourceNodeComponent& resNode) {
  resNode.Miner = 0;
}

long long ResourceNodeSystem::leftcount(ResourceNodeComponent& resNode) {
  return resNode.LeftResource;
}
