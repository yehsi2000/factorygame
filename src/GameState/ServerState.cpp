#include "GameState/ServerState.h"

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
#include "Core/GEngine.h"
#include "Core/Registry.h"
#include "Core/Server.h"
#include "Core/Packet.h"
#include "Core/ThreadSafeQueue.h"
#include "Core/TimerManager.h"
#include "Core/World.h"
#include "Core/WorldAssetManager.h"
#include "GameState/IGameState.h"
#include "GameState/MainMenuState.h"
#include "GameState/ServerState.h"
#include "System/AnimationSystem.h"
#include "System/AssemblingMachineSystem.h"
#include "System/CameraSystem.h"
#include "System/InputSystem.h"
#include "System/InteractionSystem.h"
#include "System/InventorySystem.h"
#include "System/ItemDragSystem.h"
#include "System/MiningDrillSystem.h"
#include "System/MovementSystem.h"
#include "System/ServerNetworkSystem.h"
#include "System/RefinerySystem.h"
#include "System/RenderSystem.h"
#include "System/ResourceNodeSystem.h"
#include "System/TimerExpireSystem.h"
#include "System/TimerSystem.h"
#include "System/UISystem.h"
#include "imgui_impl_sdlrenderer2.h"

ServerState::ServerState() {}

void ServerState::Init(GEngine* engine) {
  gWindow = engine->GetWindow();
  gRenderer = engine->GetRenderer();
  gFont = engine->GetFont();
  assetManager = engine->GetAssetManager();
  worldAssetManager = engine->GetWorldAssetManager();

  
  timerManager = std::make_unique<TimerManager>();
  eventDispatcher = std::make_unique<EventDispatcher>();
  registry = std::make_unique<Registry>(eventDispatcher.get());
  commandQueue = std::make_unique<CommandQueue>();
  packetQueue = std::make_unique<ThreadSafeQueue<PacketPtr>>();
  serverSendQueue = std::make_unique<ThreadSafeQueue<SendRequest>>(); // Initialize as SendRequest queue
  server = std::make_unique<Server>(); // ServerImpl needs SendRequest queue
  server->Init(packetQueue.get(), serverSendQueue.get());
  server->Start();

  entityFactory = std::make_unique<EntityFactory>(registry.get(), assetManager);
  assert(timerManager && "Fail to initialize GEngine : Invalid timer manager");
  assert(eventDispatcher &&
         "Fail to initialize GEngine : Invalid eventDispatcher");
  assert(commandQueue && "Fail to initialize GEngine : Invalid command queue");

  RegisterComponent();

  world = std::make_unique<World>(registry.get(), worldAssetManager,
                                  entityFactory.get(), eventDispatcher.get(),
                                  gFont);

  systemContext.assetManager = assetManager;
  systemContext.worldAssetManager = worldAssetManager;
  systemContext.commandQueue = commandQueue.get();
  systemContext.registry = registry.get();
  systemContext.eventDispatcher = eventDispatcher.get();
  systemContext.world = world.get();
  systemContext.inputManager = engine->GetInputManager();
  systemContext.entityFactory = entityFactory.get();
  systemContext.timerManager = timerManager.get();
  systemContext.packetQueue = packetQueue.get();
  systemContext.serverSendQueue = serverSendQueue.get(); // Pass to server-specific send queue
  systemContext.server = server.get();

  InitCoreSystem();

  world->GeneratePlayer();
  EntityID player = world->GetPlayer();

  // Default item
  eventDispatcher->Publish(
      ItemAddEvent(player, ItemID::AssemblingMachine, 100));
  eventDispatcher->Publish(ItemAddEvent(player, ItemID::MiningDrill, 100));
}

void ServerState::RegisterComponent() {
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

void ServerState::InitCoreSystem() {
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
  networkSystem = std::make_unique<ServerNetworkSystem>(systemContext);
  refinerySystem = std::make_unique<RefinerySystem>(systemContext);
  resourceNodeSystem = std::make_unique<ResourceNodeSystem>(systemContext);
  timerExpireSystem = std::make_unique<TimerExpireSystem>(systemContext);
  timerSystem = std::make_unique<TimerSystem>(systemContext);
  uiSystem = std::make_unique<UISystem>(systemContext);

  renderSystem =
      std::make_unique<RenderSystem>(systemContext, gRenderer, gFont);
}

void ServerState::Cleanup() {
  GameEndEventHandle.reset();
}

void ServerState::Update(float deltaTime) {
  SDL_GetWindowSize(gWindow, &screenSize.x, &screenSize.y);
  inputSystem->Update();
  // Process all pending commands.
  while (!commandQueue->IsEmpty()) {
    std::unique_ptr<Command> command = commandQueue->Dequeue();
    if (command) {
      command->Execute(registry.get(), eventDispatcher.get(), world.get());
    }
  }

  networkSystem->Update(deltaTime);
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

  renderSystem->Update();
  uiSystem->Update();  // Display UI on very top
}