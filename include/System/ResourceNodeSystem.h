#pragma once
 
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
