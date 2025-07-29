#include "GEngine.h"

#include <cassert>
#include <type_traits>
#include <easy/profiler.h>

#include "AssetManager.h"
#include "Components/AnimationComponent.h"
#include "Components/CameraComponent.h"
#include "Components/ChunkComponent.h"
#include "Components/InteractableComponent.h"
#include "Components/InventoryComponent.h"
#include "Components/MovableComponent.h"
#include "Components/MovementComponent.h"
#include "Components/RefineryComponent.h"
#include "Components/ResourceNodeComponent.h"
#include "Components/SpriteComponent.h"
#include "Components/TimerComponent.h"
#include "Components/TransformComponent.h"
#include "Components/TextComponent.h"
#include "Event.h"
#include "EventDispatcher.h"
#include "GameState.h"
#include "Item.h"
#include "Registry.h"
#include "SDL.h"
#include "World.h"
#include "System/AnimationSystem.h"
#include "System/CameraSystem.h"
#include "System/InventorySystem.h"
#include "System/MovementSystem.h"
#include "System/RefinerySystem.h"
#include "System/RenderSystem.h"
#include "System/ResourceNodeSystem.h"
#include "System/TimerSystem.h"

void GEngine::InitCoreSystem() {
  commandQueue = std::make_unique<CommandQueue>();
  registry = std::make_unique<Registry>();
  dispatcher = std::make_unique<EventDispatcher>();
  dispatcher->Init(GetCommandQueue());

  assert(registry && "Fail to initialize registry");

  itemDatabase = std::make_shared<ItemDatabase>();
  itemDatabase->initialize();

  animationSystem = std::make_unique<AnimationSystem>(registry.get());
  inputSystem = std::make_unique<InputSystem>(this);
  inputSystem->RegisterInputBindings();
  inventorySystem = std::make_unique<InventorySystem>(itemDatabase);
  movementSystem = std::make_unique<MovementSystem>(registry.get());
  refinerySystem = std::make_unique<RefinerySystem>(registry.get());
  renderSystem = std::make_unique<RenderSystem>(registry.get(), gRenderer, gFont);
  resourceNodeSystem =
      std::make_unique<ResourceNodeSystem>(itemDatabase, registry.get());
  timerSystem = std::make_unique<TimerSystem>(registry.get());
  timerExpireSystem = std::make_unique<TimerExpireSystem>(this);

  world = new World(this);
}

void GEngine::RegisterComponent() {
  registry->RegisterComponent<AnimationComponent>();
  registry->RegisterComponent<CameraComponent>();
  registry->RegisterComponent<ChunkComponent>();
  registry->RegisterComponent<InteractableComponent>();
  registry->RegisterComponent<InventoryComponent>();
  registry->RegisterComponent<MovableComponent>();
  registry->RegisterComponent<MovementComponent>();
  registry->RegisterComponent<RefineryComponent>();
  registry->RegisterComponent<ResourceNodeComponent>();
  registry->RegisterComponent<SpriteComponent>();
  registry->RegisterComponent<TimerComponent>();
  registry->RegisterComponent<TimerExpiredTag>();
  registry->RegisterComponent<TransformComponent>();
  registry->RegisterComponent<TextComponent>();
}

void GEngine::GeneratePlayer() {
  player = registry->CreateEntity();
  registry->EmplaceComponent<InventoryComponent>(player);

  int w, h;
  SDL_GetWindowSize(gWindow, &w, &h);
  registry->AddComponent<TransformComponent>(
      player, TransformComponent{{w / 2.f, h / 2.f}});

  SDL_Texture* playerIdleSpritesheet = AssetManager::getInstance().getTexture(
      "assets/img/character/Miner_IdleAnimation.png", gRenderer);
  registry->AddComponent<SpriteComponent>(
      player, SpriteComponent{playerIdleSpritesheet, {0, 0, 16, 16}, {0, 0, TILE_PIXEL_WIDTH, TILE_PIXEL_HEIGHT}, SDL_FLIP_NONE, render_order_t(100)});

  AnimationComponent anim;
  anim.animations["PlayerIdle"] = {0, 12, 8.f, 16, 16, true};
  anim.currentAnimationName = "PlayerIdle";
  registry->AddComponent<AnimationComponent>(player, std::move(anim));
  registry->EmplaceComponent<MovementComponent>(player);
  registry->EmplaceComponent<MovableComponent>(player);
}

GEngine::GEngine(SDL_Window* window, SDL_Renderer* renderer, TTF_Font* font)
    : gWindow(window), gRenderer(renderer), gFont(font) {

  // 레지스트리, 이벤트 디스패쳐 등 코어클래스와 시스템들 등록
  InitCoreSystem();

  // 컴포넌트 등록
  RegisterComponent();

  // 플레이어 생성 및 컴포넌트 등록
  GeneratePlayer();

  // 카메라 시스템 초기화 (플레이어 생성 후)
  cameraSystem = std::make_unique<CameraSystem>(registry.get(), player);

  // 게임 종료 이벤트 구독
  GameEndHandle =
      std::make_unique<EventHandle>(GetDispatcher()->Subscribe<QuitEvent>(
          [this](const QuitEvent&) { this->bIsRunning = false; }));
}

void GEngine::ChangeState(std::unique_ptr<GameState> newState) {
  if (currentState) currentState->Exit();
  currentState = std::move(newState);
  if (currentState) currentState->Enter();
}

void GEngine::Update(float deltaTime) {
  inputSystem->Update();
  timerSystem->Update(deltaTime);
  timerExpireSystem->Update();

  CommandQueue* cq = GetCommandQueue();
  auto commands = cq->PopAll();
  while (!commands.empty()) {
    auto command = commands.front();
    command();
    commands.pop();
  }

  world->Update(registry->GetComponent<TransformComponent>(player).position);
  movementSystem->Update(deltaTime);
  animationSystem->Update(deltaTime);
  cameraSystem->Update(deltaTime);
  // inventorySystem->Update();
  refinerySystem->Update();
  resourceNodeSystem->Update();
  
  renderSystem->Update();
}
