#ifndef CORE_SYSTEMCONTEXT_
#define CORE_SYSTEMCONTEXT_

class AssetManager;
class WorldAssetManager;
class CommandQueue;
class EntityFactory;
class EventDispatcher;
class Registry;
class TimerManager;
class World;

// TODO :
// [x]  1. De-Singleton `AssetManager`
// Make it a regular class owned by GEngine and add it to SystemContext.
// Manage its lifetime separately from the game-session objects.
// [x] 2. Beef up `EntityFactory`
// Make it a class, not just a namespace of free functions.
// Give it the SystemContext so it can access the Registry and AssetManager.
// [x]  3. Centralize Creation Logic
// Move all the "recipe" code for creating entities out of your systems
// (InputSystem, World) and into the EntityFactory.
// [x]  4. Refactor Systems
// Systems should now call the factory, e.g., m_factory->CreateWhatever(),
// instead of doing the messy work themselves.

// TODO : Also don't forget to make default destructor for systems having unique_ptr and forward declaring the type

struct SystemContext {
  AssetManager *assetManager = nullptr;
  WorldAssetManager *worldAssetManager = nullptr;
  CommandQueue *commandQueue = nullptr;
  EntityFactory *entityFactory = nullptr;
  EventDispatcher *eventDispatcher = nullptr;
  Registry *registry = nullptr;
  TimerManager *timerManager = nullptr;
  World *world = nullptr;
};

#endif /* CORE_SYSTEMCONTEXT_ */
