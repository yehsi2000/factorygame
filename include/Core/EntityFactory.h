#ifndef CORE_ENTITYFACTORY_
#define CORE_ENTITYFACTORY_

#include "Core/Entity.h"
#include "Core/Packet.h"
#include "Core/Type.h"

class Registry;
class World;
class AssetManager;

/**
 * @brief A factory for creating pre-configured entities.
 * @details Encapsulates the logic for constructing complex entities with a
 * specific set of components. This class simplifies the process of spawning
 * common game objects like players, machines, and resources, ensuring they are
 * initialized correctly.
 */
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

  virtual EntityID CreatePlayer(World *world, Vec2f worldPos, clientid_t clientID, bool bIsLocalPlayer);
};

#endif /* CORE_ENTITYFACTORY_ */
