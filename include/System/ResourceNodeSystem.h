#ifndef SYSTEM_RESOURCENODESYSTEM_
#define SYSTEM_RESOURCENODESYSTEM_

#include <memory>

#include "Components/ResourceNodeComponent.h"
#include "Entity.h"
#include "Item.h"
#include "Registry.h"

class ResourceNodeSystem {
 public:
  ResourceNodeSystem(std::shared_ptr<ItemDatabase> db, Registry* r);
  void Update();
  void AddMiner(ResourceNodeComponent& resNode, EntityID player);
  void RemoveMiner(ResourceNodeComponent& resNode);
  long long leftcount(ResourceNodeComponent& resNode);
  ~ResourceNodeSystem();

 private:
  std::shared_ptr<ItemDatabase> itemDatabase;
  Registry* registry;
};

#endif /* SYSTEM_RESOURCENODESYSTEM_ */
