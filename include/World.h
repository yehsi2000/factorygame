#pragma once

#include <memory>
#include <utility>
#include <vector>
#include <cassert>

// #include "ComponentManager.h"
#include "Event.h"
#include "Registry.h"
#include "GameState.h"
#include "Item.h"
#include "InventoryComponent.h"
#include "InventorySystem.h"
#include "RefineryComponent.h"
#include "RefinerySystem.h"
#include "ResourceNodeComponent.h"
#include "ResourceNodeSystem.h"

class World {
  std::unique_ptr<EventDispatcher> dispatcher;
  EntityID player;
  std::vector<EntityID> entities;
  std::unique_ptr<GameState> currentState;

  EntityID nextID = 1;

  World(const World&) = delete;
  World& operator=(const World&) = delete;
  World(World&&) = delete;
  World& operator=(World&&) = delete;

 public:
  // ComponentManager<ResourceNodeComponent> resourceNodes;
  // ComponentManager<RefineryComponent> refineryManager;
  // ComponentManager<InventoryComponent> inventoryManager;
  std::shared_ptr<ItemDatabase> itemDatabase;
  std::unique_ptr<ResourceNodeSystem> resourceNodeSystem;
  std::unique_ptr<RefinerySystem> refinerySystem;
  std::unique_ptr<InventorySystem> inventorySystem;
  std::unique_ptr<Registry> registry;
  

  World() {
    itemDatabase = std::make_shared<ItemDatabase>();
    resourceNodeSystem = std::make_unique<ResourceNodeSystem>(itemDatabase);
    refinerySystem = std::make_unique<RefinerySystem>();
    inventorySystem = std::make_unique<InventorySystem>(itemDatabase);
    dispatcher = std::make_unique<EventDispatcher>();
    registry = std::make_unique<Registry>();
    assert(registry && "Fail to initialize registry");
    registry->registerComponent<InventoryComponent>();
    registry->registerComponent<ResourceNodeComponent>();
    registry->registerComponent<RefineryComponent>();
    player = registry->createEntity();
  }

  void ChangeState(std::unique_ptr<GameState> newState);
  void Update();

  inline EventDispatcher* GetDispatcher() { return dispatcher.get(); }
  inline EntityID GetPlayer() { return player; }
};