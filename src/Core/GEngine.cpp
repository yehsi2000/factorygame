#include "Core/GEngine.h"

#include <cassert>
#include <chrono>
#include <tuple>
#include <utility>

#include "Components/AnimationComponent.h"
#include "Components/AssemblingMachineComponent.h"
#include "Components/BuildingComponent.h"
#include "Components/BuildingPreviewComponent.h"
#include "Components/CameraComponent.h"
#include "Components/ChunkComponent.h"
#include "Components/DebugRectComponent.h"
#include "Components/InactiveComponent.h"
#include "Components/InventoryComponent.h"
#include "Components/MiningDrillComponent.h"
#include "Components/MovableComponent.h"
#include "Components/MovementComponent.h"
#include "Components/PlayerStateComponent.h"
#include "Components/RefineryComponent.h"
#include "Components/ResourceNodeComponent.h"
#include "Components/SpriteComponent.h"
#include "Components/TextComponent.h"
#include "Components/TimerComponent.h"
#include "Components/TransformComponent.h"
#include "Core/AssetManager.h"
#include "Core/CommandQueue.h"
#include "Core/EntityFactory.h"
#include "Core/Event.h"
#include "Core/EventDispatcher.h"
#include "Core/IGameState.h"
#include "Core/InputPoller.h"
#include "Core/Registry.h"
#include "Core/TimerManager.h"
#include "Core/World.h"
#include "Core/WorldAssetManager.h"
#include "GameState/MainMenuState.h"
#include "GameState/PlayState.h"
#include "System/AnimationSystem.h"
#include "System/AssemblingMachineSystem.h"
#include "System/CameraSystem.h"
#include "System/InputSystem.h"
#include "System/InteractionSystem.h"
#include "System/InventorySystem.h"
#include "System/ItemDragSystem.h"
#include "System/MiningDrillSystem.h"
#include "System/MovementSystem.h"
#include "System/RefinerySystem.h"
#include "System/RenderSystem.h"
#include "System/ResourceNodeSystem.h"
#include "System/TimerExpireSystem.h"
#include "System/TimerSystem.h"
#include "System/UISystem.h"
#include "imgui_impl_sdlrenderer2.h"

void GEngine::InitCoreSystem() {
  animationSystem = std::make_unique<AnimationSystem>(systemContext);
  assemblingMachineSystem =
      std::make_unique<AssemblingMachineSystem>(systemContext);
  cameraSystem = std::make_unique<CameraSystem>(systemContext);
  inputSystem = std::make_unique<InputSystem>(systemContext);
  interactionSystem = std::make_unique<InteractionSystem>(systemContext);
  inventorySystem = std::make_unique<InventorySystem>(systemContext);
  itemDragSystem = std::make_unique<ItemDragSystem>(systemContext);
  miningDrillSystem = std::make_unique<MiningDrillSystem>(systemContext);
  movementSystem = std::make_unique<MovementSystem>(systemContext);
  refinerySystem = std::make_unique<RefinerySystem>(systemContext);
  resourceNodeSystem = std::make_unique<ResourceNodeSystem>(systemContext);
  timerExpireSystem = std::make_unique<TimerExpireSystem>(systemContext);
  timerSystem = std::make_unique<TimerSystem>(systemContext);
  uiSystem = std::make_unique<UISystem>(systemContext);

  renderSystem =
      std::make_unique<RenderSystem>(systemContext, gRenderer, gFont);
}

void GEngine::RegisterComponent() {
  // Register all component type inside typeArray to regsitry
  // powered by Lambda TMP Magic™
  using ComponentTypes =
      typeArray<AnimationComponent, AssemblingMachineComponent,
                BuildingComponent, BuildingPreviewComponent, CameraComponent,
                ChunkComponent, DebugRectComponent, InactiveComponent,
                InventoryComponent, MiningDrillComponent, MovableComponent,
                MovementComponent, PlayerStateComponent, RefineryComponent,
                ResourceNodeComponent, SpriteComponent, TimerComponent,
                TimerExpiredTag, TransformComponent, TextComponent>;

  [reg = registry.get()]<std::size_t... Is>(std::index_sequence<Is...>) {
    ((reg->RegisterComponent<
         std::tuple_element_t<Is, typename ComponentTypes::typesTuple>>()),
     ...);
  }(std::make_index_sequence<ComponentTypes::size>{});
}

GEngine::GEngine(SDL_Window *window, SDL_Renderer *renderer, TTF_Font *font)
    : gWindow(window),
      gRenderer(renderer),
      gFont(font),
      assetManager(std::make_unique<AssetManager>(renderer)),
      worldAssetManager(std::make_unique<WorldAssetManager>(renderer)),
      registry(std::make_unique<Registry>()),
      timerManager(std::make_unique<TimerManager>()),
      eventDispatcher(std::make_unique<EventDispatcher>()),
      commandQueue(std::make_unique<CommandQueue>()),
      inputPoller(std::make_unique<InputPoller>(gWindow)),
      GameEndEventHandle(nullptr),
      entityFactory(
          std::make_unique<EntityFactory>(registry.get(), assetManager.get())) {
  // core class initialization should not fail
  assert(timerManager && "Fail to initialize GEngine : Invalid timer manager");
  assert(eventDispatcher &&
         "Fail to initialize GEngine : Invalid eventDispatcher");
  assert(commandQueue && "Fail to initialize GEngine : Invalid command queue");

  GameEndEventHandle = eventDispatcher->Subscribe<QuitEvent>(
      [this](const QuitEvent &) { bIsRunning = false; });

  // TODO : Transit to State Design Pattern
  currentState = std::make_unique<PlayState>();

  RegisterComponent();

  // TODO : Move world generation logic into another class and world should be data
  world =
      std::make_unique<World>(registry.get(), worldAssetManager.get(),
                              entityFactory.get(), eventDispatcher.get(), font);

  systemContext.assetManager = assetManager.get();
  systemContext.worldAssetManager = worldAssetManager.get();
  systemContext.commandQueue = commandQueue.get();
  systemContext.registry = registry.get();
  systemContext.eventDispatcher = eventDispatcher.get();
  systemContext.world = world.get();
  systemContext.inputPoller = inputPoller.get();
  systemContext.entityFactory = entityFactory.get();
  systemContext.timerManager = timerManager.get();
  InitCoreSystem();

  EntityID player = world->GetPlayer();

  // Default item
  eventDispatcher->Publish(
      ItemAddEvent(player, ItemID::AssemblingMachine, 100));
  eventDispatcher->Publish(ItemAddEvent(player, ItemID::MiningDrill, 100));
}

GEngine::~GEngine() = default;

void GEngine::ChangeState(std::unique_ptr<IGameState> newState) {
  if (currentState) currentState->Cleanup();
  currentState = std::move(newState);
  if (currentState) currentState->Init(this);
}

void GEngine::Run() {
  using namespace std::chrono;

  steady_clock::time_point startTime;
  steady_clock::time_point curTime;
  steady_clock::time_point prevTime;
  float deltaTime;

  startTime = steady_clock::now();
  curTime = prevTime = startTime;

  while (bIsRunning) {
    curTime = steady_clock::now();
    deltaTime =
        duration<float, milliseconds::period>(curTime - prevTime).count();
    deltaTime /= 1000.f;
    prevTime = curTime;

    Update(deltaTime);
  }
}

void GEngine::Update(float deltaTime) {
  inputPoller->PollEvents();
  // Process all pending commands.
  while (!commandQueue->IsEmpty()) {
    std::unique_ptr<Command> command = commandQueue->Dequeue();
    if (command) {
      command->Execute(registry.get(), eventDispatcher.get(), world.get());
    }
  }
  inputSystem->Update();
  itemDragSystem->Update();
  timerSystem->Update(deltaTime);
  timerExpireSystem->Update();
  interactionSystem->Update();

  world->Update();
  movementSystem->Update(deltaTime);
  animationSystem->Update(deltaTime);
  assemblingMachineSystem->Update();
  miningDrillSystem->Update();
  refinerySystem->Update();
  resourceNodeSystem->Update();

  cameraSystem->Update(deltaTime);

  // Rendering
  renderSystem->Update();
  uiSystem->Update();  // Display UI on very top
  ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), gRenderer);
  SDL_RenderPresent(gRenderer);
}