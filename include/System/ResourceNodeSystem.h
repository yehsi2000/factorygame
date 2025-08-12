#ifndef SYSTEM_RESOURCENODESYSTEM_
#define SYSTEM_RESOURCENODESYSTEM_

class Registry;
class World;

class ResourceNodeSystem {
 public:
  ResourceNodeSystem(Registry* r, World* world);
  void Update();

 private:
  Registry* registry;
  World* world;
};

#endif /* SYSTEM_RESOURCENODESYSTEM_ */