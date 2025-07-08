#include "GEngine.h"

#include <cassert>
#include <type_traits>

#include "AssetManager.h"
#include "Components/AnimationComponent.h"
#include "Components/InteractableComponent.h"
#include "Components/InventoryComponent.h"
#include "Components/MovementComponent.h"
#include "Components/RefineryComponent.h"
#include "Components/ResourceNodeComponent.h"
#include "Components/SpriteComponent.h"
#include "Components/TimerComponent.h"
#include "Components/TransformComponent.h"
#include "Event.h"
#include "GameState.h"
#include "Item.h"
#include "Registry.h"
#include "SDL.h"
#include "System/AnimationSystem.h"
#include "System/InventorySystem.h"
#include "System/MovementSystem.h"
#include "System/RefinerySystem.h"
#include "System/RenderSystem.h"
#include "System/ResourceNodeSystem.h"
#include "System/TimerSystem.h"

void GEngine::InitCoreSystem() {
  registry = std::make_unique<Registry>();
  dispatcher = std::make_unique<EventDispatcher>();

  assert(registry && "Fail to initialize registry");

  itemDatabase = std::make_shared<ItemDatabase>();
  itemDatabase->initialize();

  animationSystem = std::make_unique<AnimationSystem>(registry.get());
  inventorySystem = std::make_unique<InventorySystem>(itemDatabase);
  movementSystem = std::make_unique<MovementSystem>(registry.get());
  refinerySystem = std::make_unique<RefinerySystem>(registry.get());
  renderSystem = std::make_unique<RenderSystem>(registry.get(), gRenderer);
  resourceNodeSystem =
      std::make_unique<ResourceNodeSystem>(itemDatabase, registry.get());
  timerSystem = std::make_unique<TimerSystem>(registry.get());
}

void GEngine::RegisterComponent() {
  registry->registerComponent<AnimationComponent>();
  registry->registerComponent<InteractableComponent>();
  registry->registerComponent<InventoryComponent>();
  registry->registerComponent<MovementComponent>();
  registry->registerComponent<RefineryComponent>();
  registry->registerComponent<ResourceNodeComponent>();
  registry->registerComponent<SpriteComponent>();
  registry->registerComponent<TimerComponent>();
  registry->registerComponent<TransformComponent>();
}

void GEngine::GeneratePlayer() {
  player = registry->createEntity();
  registry->emplaceComponent<InventoryComponent>(player);

  int w, h;
  SDL_GetWindowSize(gWindow, &w, &h);
  registry->addComponent<TransformComponent>(
      player, TransformComponent{w / 2.f, h / 2.f, 5.f, 5.f});

  SDL_Texture* playerIdleSpritesheet = AssetManager::getInstance().getTexture(
      "assets/img/character/Miner_IdleAnimation.png", gRenderer);
  registry->addComponent<SpriteComponent>(
      player, SpriteComponent{playerIdleSpritesheet, {0, 0, 16, 16}});

  AnimationComponent anim;
  anim.animations["PlayerIdle"] = {0, 12, 8.f, 16, 16, true};
  anim.currentAnimationName = "PlayerIdle";
  registry->addComponent<AnimationComponent>(player, std::move(anim));

  registry->emplaceComponent<MovementComponent>(player);

  MovementComponent& pMove = registry->getComponent<MovementComponent>(player);
  dispatcher->Subscribe<XAxisEvent>(
      [&pMove](XAxisEvent e) { pMove.dx = e.val; });
  dispatcher->Subscribe<YAxisEvent>(
      [&pMove](YAxisEvent e) { pMove.dy = e.val; });
}

GEngine::GEngine(SDL_Window* window, SDL_Renderer* renderer) {
  gWindow = window;
  gRenderer = renderer;

  // 레지스트리, 이벤트 디스패쳐 등 코어클래스와 시스템들 등록
  InitCoreSystem();

  // 컴포넌트 등록
  RegisterComponent();

  // 플레이어 생성 및 컴포넌트 등록
  GeneratePlayer();
}

void GEngine::ChangeState(std::unique_ptr<GameState> newState) {
  if (currentState) currentState->Exit();
  currentState = std::move(newState);
  if (currentState) currentState->Enter();
}

void GEngine::Update(float deltaTime) {
  timerSystem->Update(deltaTime);

  animationSystem->Update(deltaTime);
  // inventorySystem->Update();
  refinerySystem->Update();
  resourceNodeSystem->Update();

  movementSystem->Update(deltaTime);

  renderSystem->Update();
}
