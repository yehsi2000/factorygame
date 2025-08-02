#include "Core/GEngine.h"

#include <easy/profiler.h>

#include <cassert>
#include <type_traits>

#include "Components/AnimationComponent.h"
#include "Components/CameraComponent.h"
#include "Components/ChunkComponent.h"
#include "Components/InactiveComponent.h"
#include "Components/InteractionComponent.h"
#include "Components/InventoryComponent.h"
#include "Components/MovableComponent.h"
#include "Components/MovementComponent.h"
#include "Components/RefineryComponent.h"
#include "Components/ResourceNodeComponent.h"
#include "Components/SpriteComponent.h"
#include "Components/TextComponent.h"
#include "Components/TimerComponent.h"
#include "Components/TransformComponent.h"
#include "Core/AssetManager.h"
#include "Core/Event.h"
#include "Core/EventDispatcher.h"
#include "Core/GameState.h"
#include "Core/Item.h"
#include "Core/Registry.h"
#include "Core/TimerManager.h"
#include "Core/World.h"
#include "SDL.h"
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
  timerManager = std::make_unique<TimerManager>();
  world = std::make_unique<World>(gRenderer, registry.get(), gFont);

  assert(registry && "Fail to initialize registry");

  animationSystem = std::make_unique<AnimationSystem>(registry.get());
  inputSystem = std::make_unique<InputSystem>(this);
  inputSystem->RegisterInputBindings();
  inventorySystem = std::make_unique<InventorySystem>();
  movementSystem =
      std::make_unique<MovementSystem>(registry.get(), timerManager.get());
  refinerySystem = std::make_unique<RefinerySystem>(registry.get());
  renderSystem =
      std::make_unique<RenderSystem>(registry.get(), gRenderer, gFont);
  resourceNodeSystem = std::make_unique<ResourceNodeSystem>(registry.get());
  timerSystem =
      std::make_unique<TimerSystem>(registry.get(), timerManager.get());
  timerExpireSystem = std::make_unique<TimerExpireSystem>(this);
  interactionSystem = std::make_unique<InteractionSystem>(
      registry.get(), world.get(), dispatcher.get(), commandQueue.get());
}

void GEngine::RegisterComponent() {
  registry->RegisterComponent<AnimationComponent>();
  registry->RegisterComponent<CameraComponent>();
  registry->RegisterComponent<ChunkComponent>();
  registry->RegisterComponent<InactiveComponent>();
  registry->RegisterComponent<InventoryComponent>();
  registry->RegisterComponent<MovableComponent>();
  registry->RegisterComponent<InteractionComponent>();
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
  registry->AddComponent<TransformComponent>(player,
                                             TransformComponent{{0, 0}});

  SDL_Texture* playerIdleSpritesheet = AssetManager::getInstance().getTexture(
      "assets/img/character/Miner_IdleAnimation.png", gRenderer);
  registry->AddComponent<SpriteComponent>(
      player, SpriteComponent{playerIdleSpritesheet,
                              {0, 0, 16, 16},
                              {TILE_PIXEL_WIDTH / 2, TILE_PIXEL_HEIGHT / 2,
                               TILE_PIXEL_WIDTH, TILE_PIXEL_HEIGHT},
                              SDL_FLIP_NONE,
                              render_order_t(100)});

  AnimationComponent anim;
  anim.animations["PlayerIdle"] = {0, 12, 8.f, 16, 16, true};
  anim.currentAnimationName = "PlayerIdle";
  registry->AddComponent<AnimationComponent>(player, std::move(anim));
  registry->EmplaceComponent<MovementComponent>(player);
  registry->EmplaceComponent<MovableComponent>(player);
  registry->EmplaceComponent<InventoryComponent>(player);
}

GEngine::GEngine(SDL_Window* window, SDL_Renderer* renderer, TTF_Font* font)
    : gWindow(window), gRenderer(renderer), gFont(font) {
  // Register core classes and systems such as the registry and event dispatcher
  InitCoreSystem();
  RegisterComponent();
  GeneratePlayer();
  cameraSystem = std::make_unique<CameraSystem>(registry.get(), player);
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
  // Process all pending commands.
  while (!commandQueue->IsEmpty()) {
    std::unique_ptr<Command> command = commandQueue->Dequeue();
    if (command) {
      command->Execute(*this, *registry);
    }
  }

  inputSystem->Update();
  timerSystem->Update(deltaTime);
  timerExpireSystem->Update();
  interactionSystem->Update();
  world->Update(player);
  movementSystem->Update(deltaTime);
  animationSystem->Update(deltaTime);
  cameraSystem->Update(deltaTime);
  refinerySystem->Update();
  resourceNodeSystem->Update();

  renderSystem->Update();
}
