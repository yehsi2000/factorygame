#ifndef SYSTEM_RESOURCENODESYSTEM_
#define SYSTEM_RESOURCENODESYSTEM_

#include <memory>

#include "Components/ResourceNodeComponent.h"
#include "Core/Entity.h"
#include "Core/Item.h"
#include "Core/Registry.h"

class ResourceNodeSystem {
 public:
  ResourceNodeSystem(std::shared_ptr<ItemDatabase> db, Registry* r);
  void Update();
  long long leftcount(ResourceNodeComponent& resNode);
  ~ResourceNodeSystem();

 private:
  std::shared_ptr<ItemDatabase> itemDatabase;
  Registry* registry;
};

#endif /* SYSTEM_RESOURCENODESYSTEM_ */
