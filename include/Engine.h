#include <cassert>
#include <memory>
#include <utility>
#include <vector>

#include "Entity.h"

class EventDispatcher;
class GameState;
class ItemDatabase;
class ResourceNodeSystem;
class RefinerySystem;
class InventorySystem;
class TimerSystem;
class Registry;

class Engine {
  std::unique_ptr<EventDispatcher> dispatcher;
  EntityID player;
  std::vector<EntityID> entities;
  std::unique_ptr<GameState> currentState;
  std::shared_ptr<ItemDatabase> itemDatabase;
  std::unique_ptr<ResourceNodeSystem> resourceNodeSystem;
  std::unique_ptr<RefinerySystem> refinerySystem;
  std::unique_ptr<InventorySystem> inventorySystem;
  std::unique_ptr<TimerSystem> timerSystem;
  std::unique_ptr<Registry> registry;

  EntityID nextID = 1;

  Engine(const Engine&) = delete;
  Engine& operator=(const Engine&) = delete;
  Engine(Engine&&) = delete;
  Engine& operator=(Engine&&) = delete;

 public:
  Engine();
  ~Engine();

  void ChangeState(std::unique_ptr<GameState> newState);
  void Update(double deltaTime);

  inline EventDispatcher* GetDispatcher() { return dispatcher.get(); }
  inline EntityID GetPlayer() { return player; }
  inline Registry* GetRegistry() { return registry.get(); }
};