#ifndef SYSTEM_RESOURCENODESYSTEM_
#define SYSTEM_RESOURCENODESYSTEM_

#include "Components/ResourceNodeComponent.h"
#include "Core/Registry.h"

class ResourceNodeSystem {
 public:
  ResourceNodeSystem(Registry* r);
  void Update();

 private:
  Registry* registry;
};

#endif /* SYSTEM_RESOURCENODESYSTEM_ */
