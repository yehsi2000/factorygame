#include "GameState/ServerState.h"

#include <cassert>

#include "Core/CommandQueue.h"
#include "Core/EntityFactory.h"
#include "Core/Event.h"
#include "Core/EventDispatcher.h"
#include "Core/GEngine.h"
#include "Core/Packet.h"
#include "Core/Registry.h"
#include "Core/Server.h"
#include "DataStruct/ThreadSafeQueue.h"
#include "Core/TimerManager.h"
#include "Core/World.h"
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
#include "System/RefinerySystem.h"
#include "System/RenderSystem.h"
#include "System/ResourceNodeSystem.h"
#include "System/ServerNetworkSystem.h"
#include "System/TimerExpireSystem.h"
#include "System/TimerSystem.h"
#include "System/UISystem.h"

ServerState::ServerState() {}
ServerState::~ServerState() = default;

void ServerState::Init(GEngine* engine) {
  gEngine = engine;
  gWindow = engine->GetWindow();
  gRenderer = engine->GetRenderer();
  gFont = engine->GetFont();
  assetManager = engine->GetAssetManager();
  worldAssetManager = engine->GetWorldAssetManager();

  timerManager = std::make_unique<TimerManager>();
  eventDispatcher = std::make_unique<EventDispatcher>();
  registry = std::make_unique<Registry>(eventDispatcher.get());
  commandQueue = std::make_unique<CommandQueue>();
  recvQueue = std::make_unique<ThreadSafeQueue<RecvPacketPtr>>();
  sendQueue = std::make_unique<ThreadSafeQueue<SendRequestPtr>>();

  pendingMoves = std::make_unique<ThreadSafeQueue<MoveAppliedPtr>>();
  server = std::make_unique<Server>();  // ServerImpl needs SendRequestPtr queue
  server->Init(recvQueue.get(), sendQueue.get());
  server->Start();
  std::string serverName = "Server";
  clientNameMap[0] = serverName;

  entityFactory = std::make_unique<EntityFactory>(registry.get(), assetManager);
  assert(timerManager && "Fail to initialize GEngine : Invalid timer manager");
  assert(eventDispatcher &&
         "Fail to initialize GEngine : Invalid eventDispatcher");
  assert(commandQueue && "Fail to initialize GEngine : Invalid command queue");

  world = std::make_unique<World>(registry.get(), worldAssetManager,
                                  entityFactory.get(), eventDispatcher.get(),
                                  gFont, true);

  systemContext.assetManager = assetManager;
  systemContext.worldAssetManager = worldAssetManager;
  systemContext.commandQueue = commandQueue.get();
  systemContext.registry = registry.get();
  systemContext.eventDispatcher = eventDispatcher.get();
  systemContext.world = world.get();
  systemContext.inputManager = engine->GetInputManager();
  systemContext.entityFactory = entityFactory.get();
  systemContext.timerManager = timerManager.get();
  systemContext.serverRecvQueue = recvQueue.get();
  systemContext.pendingMoves = pendingMoves.get();
  systemContext.serverSendQueue =
      sendQueue.get();  // Pass to server-specific send queue
  systemContext.server = server.get();
  systemContext.clientNameMap = &clientNameMap;
  systemContext.isServer = true;

  InitCoreSystem();

  eventDispatcher->Subscribe<QuitEvent>(
      [this](const QuitEvent& e) { isQuit = true; });

  // TODO : Move Server player generation to be handled by menu ui
  world->GeneratePlayer(0, {0.f, 0.f}, true);
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

void ServerState::Cleanup() {}

void ServerState::Update(float deltaTime) {
  if (isQuit) {
    if (!gEngine->IsChangeRequested())
      gEngine->ChangeState(std::make_unique<MainMenuState>());
    return;  // The state is now being destroyed, so we should not continue.
  }

  inputSystem->Update();

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

  // Process all pending commands.
  while (!commandQueue->IsEmpty()) {
    std::unique_ptr<Command> command = commandQueue->Dequeue();
    if (command) {
      command->Execute(registry.get(), eventDispatcher.get(), world.get());
    }
  }

  renderSystem->Update();
  uiSystem->Update();  // Display UI on very top
}