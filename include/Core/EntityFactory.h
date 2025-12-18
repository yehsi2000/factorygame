#pragma once
 
#include "Core/Entity.h"
#include "Core/Packet.h"
#include "DataStruct/Type.h"

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
  
  virtual Entity CreateAssemblingMachine(World* world, Vec2f worldPos);
  virtual Entity CreateAssemblingMachine(World* world, Vec2 tileIndex);

  virtual Entity CreateMiningDrill(World* world, Vec2f worldPos);
  virtual Entity CreateMiningDrill(World* world, Vec2 tileIndex);

  virtual Entity CreatePlayer(World *world, Vec2f worldPos, clientid_t clientID, bool bIsLocalPlayer);
};
