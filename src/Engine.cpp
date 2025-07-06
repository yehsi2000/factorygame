#include "Engine.h"

#include "Event.h"
#include "GameState.h"
#include "InteractableComponent.h"
#include "InventoryComponent.h"
#include "InventorySystem.h"
#include "Item.h"
#include "PositionComponent.h"
#include "RefineryComponent.h"
#include "RefinerySystem.h"
#include "Registry.h"
#include "ResourceNodeComponent.h"
#include "ResourceNodeSystem.h"
#include "TimerComponent.h"
#include "TimerSystem.h"

Engine::Engine() {
  registry = std::make_unique<Registry>();
  itemDatabase = std::make_shared<ItemDatabase>();
  itemDatabase->initialize();
  resourceNodeSystem =
      std::make_unique<ResourceNodeSystem>(itemDatabase, registry.get());
  refinerySystem = std::make_unique<RefinerySystem>(registry.get());
  inventorySystem = std::make_unique<InventorySystem>(itemDatabase);
  dispatcher = std::make_unique<EventDispatcher>();
  timerSystem = std::make_unique<TimerSystem>(registry.get());

  assert(registry && "Fail to initialize registry");

  // 컴포넌트 등록
  registry->registerComponent<InventoryComponent>();
  registry->registerComponent<ResourceNodeComponent>();
  registry->registerComponent<RefineryComponent>();
  registry->registerComponent<PositionComponent>();
  registry->registerComponent<InteractableComponent>();
  registry->registerComponent<TimerComponent>();

  // 플레이어 생성 및 컴포넌트 등록
  player = registry->createEntity();
  registry->addComponent<InventoryComponent>(player);
  registry->addComponent<PositionComponent>(player);
}

Engine::~Engine() = default;

void Engine::ChangeState(std::unique_ptr<GameState> newState) {
  if (currentState) currentState->Exit();
  currentState = std::move(newState);
  if (currentState) currentState->Enter();
}

void Engine::Update(double deltaTime) {
  resourceNodeSystem->Update();
  refinerySystem->Update();
  timerSystem->Update(deltaTime);
}
