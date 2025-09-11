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
  virtual ~EntityFactory();
  
  virtual EntityID CreateAssemblingMachine(World* world, Vec2f worldPos);
  virtual EntityID CreateAssemblingMachine(World* world, Vec2 tileIndex);

  virtual EntityID CreateMiningDrill(World* world, Vec2f worldPos);
  virtual EntityID CreateMiningDrill(World* world, Vec2 tileIndex);

  virtual EntityID CreatePlayer(World* world, Vec2f worldPos);
};

#endif /* CORE_ENTITYFACTORY_ */
