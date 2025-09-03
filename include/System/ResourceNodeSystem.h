#ifndef SYSTEM_RESOURCENODESYSTEM_
#define SYSTEM_RESOURCENODESYSTEM_

#include "Core/SystemContext.h"

/**
 * @brief Responsible for updating every resource node's info inside the world
 * 
 */
class ResourceNodeSystem {
 public:
  ResourceNodeSystem(const SystemContext& context);
  ~ResourceNodeSystem();
  void Update();

 private:
  Registry* registry;
  World* world;
};

#endif /* SYSTEM_RESOURCENODESYSTEM_ */