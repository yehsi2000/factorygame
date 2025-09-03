#ifndef CORE_ENTITYFACTORY_
#define CORE_ENTITYFACTORY_

#include "Core/Entity.h"
#include "Core/Type.h"

class Registry;
class World;
class AssetManager;

class EntityFactory {
  Registry* registry;
  AssetManager* assetManager;

 public:
  EntityFactory(Registry* registry, AssetManager* assetManager);
  ~EntityFactory();
  
  EntityID CreateAssemblingMachine(World* world, Vec2f worldPos);
  EntityID CreateAssemblingMachine(World* world, Vec2 tileIndex);

  EntityID CreateMiningDrill(World* world, Vec2f worldPos);
  EntityID CreateMiningDrill(World* world, Vec2 tileIndex);

  EntityID CreatePlayer(World* world, Vec2f worldPos);
};

#endif /* CORE_ENTITYFACTORY_ */
