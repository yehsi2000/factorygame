#pragma once
#include "Item.h"
#include "ResourceNodeComponent.h"

class ResourceNodeSystem {

 public:
  ResourceNodeSystem(std::shared_ptr<ItemDatabase> db)
      : itemDatabase(db) {}
  void Update(class World& world);
  void AddMiner(ResourceNodeComponent& resNode, EntityID player);
  void RemoveMiner(ResourceNodeComponent& resNode);
  long long leftcount(ResourceNodeComponent& resNode);
  private:
    std::shared_ptr<ItemDatabase> itemDatabase;
};