#include "ResourceNodeSystem.h"

#include "ResourceNodeComponent.h"
#include "World.h"

void ResourceNodeSystem::Update(World& world) {
  //TODO : Interact Event가 발생했을 때 1초가 지날 때 마다 연결된 Miner에 ResourceNodeComponent의 Ore를 추가
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
