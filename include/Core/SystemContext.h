#ifndef CORE_SYSTEMCONTEXT_
#define CORE_SYSTEMCONTEXT_

class AssetManager;
class WorldAssetManager;
class CommandQueue;
class EntityFactory;
class EventDispatcher;
class Registry;
class TimerManager;
class InputPoller;
class World;

struct SystemContext {
  AssetManager *assetManager = nullptr;
  WorldAssetManager *worldAssetManager = nullptr;
  CommandQueue *commandQueue = nullptr;
  EntityFactory *entityFactory = nullptr;
  EventDispatcher *eventDispatcher = nullptr;
  Registry *registry = nullptr;
  TimerManager *timerManager = nullptr;
  InputPoller* inputPoller = nullptr;
  World *world = nullptr;
};

#endif /* CORE_SYSTEMCONTEXT_ */
