#include "System/ResourceNodeSystem.h"

#include "Components/ResourceNodeComponent.h"
#include "Item.h"
#include "Registry.h"

ResourceNodeSystem::ResourceNodeSystem(std::shared_ptr<ItemDatabase> db,
                                       Registry* r) {
  itemDatabase = db;
  registry = r;
}

void ResourceNodeSystem::Update() {
  // TODO : Interact Event가 발생했을 때 1초가 지날 때 마다 연결된 Miner에
  // ResourceNodeComponent의 Ore를 추가
  if (registry == nullptr) return;
}

long long ResourceNodeSystem::leftcount(ResourceNodeComponent& resNode) {
  return resNode.LeftResource;
}

ResourceNodeSystem::~ResourceNodeSystem() = default;
