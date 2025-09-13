#include "GameState/ClientState.h"

#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstring>
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
#include "Components/InputStateComponent.h"
#include "Components/InventoryComponent.h"
#include "Components/LocalPlayerComponent.h"
#include "Components/MiningDrillComponent.h"
#include "Components/MovableComponent.h"
#include "Components/MovementComponent.h"
#include "Components/NetPredictionComponent.h"
#include "Components/PlayerStateComponent.h"
#include "Components/RefineryComponent.h"
#include "Components/ResourceNodeComponent.h"
#include "Components/InterpBufferComponent.h"
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
#include "Core/Packet.h"
#include "Core/Registry.h"
#include "Core/Socket.h"
#include "Core/ThreadSafeQueue.h"
#include "Core/TimerManager.h"
#include "Core/World.h"
#include "Core/WorldAssetManager.h"
#include "GameState/IGameState.h"
#include "GameState/MainMenuState.h"
#include "System/AnimationSystem.h"
#include "System/AssemblingMachineSystem.h"
#include "System/CameraSystem.h"
#include "System/ClientNetworkSystem.h"
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


ClientState::ClientState() : gEngine(nullptr), bIsQuit(false) {}

void ClientState::Init(GEngine* engine) {
  gEngine = engine;
  gWindow = engine->GetWindow();
  gRenderer = engine->GetRenderer();
  gFont = engine->GetFont();
  assetManager = engine->GetAssetManager();
  worldAssetManager = engine->GetWorldAssetManager();

  eventDispatcher = std::make_unique<EventDispatcher>();
  registry = std::make_unique<Registry>(eventDispatcher.get());
  timerManager = std::make_unique<TimerManager>();
  commandQueue = std::make_unique<CommandQueue>();
  recvQueue = std::make_unique<ThreadSafeQueue<PacketPtr>>();
  sendQueue =
      std::make_unique<ThreadSafeQueue<PacketPtr>>();  // Initialize as
                                                       // PacketPtr queue

  entityFactory = std::make_unique<EntityFactory>(registry.get(), assetManager);
  assert(timerManager && "Fail to initialize GEngine : Invalid timer manager");
  assert(eventDispatcher &&
         "Fail to initialize GEngine : Invalid eventDispatcher");
  assert(commandQueue && "Fail to initialize GEngine : Invalid command queue");

  RegisterComponent();

  world = std::make_unique<World>(registry.get(), worldAssetManager,
                                  entityFactory.get(), eventDispatcher.get(),
                                  gFont, false);

  systemContext.assetManager = assetManager;
  systemContext.worldAssetManager = worldAssetManager;
  systemContext.commandQueue = commandQueue.get();
  systemContext.registry = registry.get();
  systemContext.eventDispatcher = eventDispatcher.get();
  systemContext.world = world.get();
  systemContext.inputManager = engine->GetInputManager();
  systemContext.entityFactory = entityFactory.get();
  systemContext.timerManager = timerManager.get();
  systemContext.clientRecvQueue = recvQueue.get();
  systemContext.clientSendQueue = sendQueue.get();
  systemContext.socket = connectionSocket.get();
  systemContext.clientNameMap = &clientNameMap;
  systemContext.bIsServer = false;
  InitCoreSystem();

  GameEndEventHandle =
      eventDispatcher->Subscribe<QuitEvent>([this](QuitEvent e) {
        bIsQuit = true;  // Signal the main thread to quit
      });

  // TODO : move message buffer and receiving thread to network system
  messageBuffer = std::vector<uint8_t>(MAX_BUFFER);

  bIsReceiving = true;
  messageThread = std::thread([this] { SocketReceiveWorker(); });
  networkSystem->Init(u8"Client");
}

bool ClientState::TryConnect() {
  connectionSocket = std::make_unique<Socket>();
  connectionSocket->Init();

  int res = connectionSocket->Connect("127.0.0.1", 27015);
  // TODO : send duplicate name check packet and return if duplicate name exists

  if (res == 0) return false;
  return true;
}

void ClientState::SocketReceiveWorker() {
  while (bIsReceiving) {
    int res =
        connectionSocket->Receive(messageBuffer.data(), messageBuffer.size());

    if (res == 0) {
      // connection closed
      std::cout << "Connection closed by server.\n";
      bIsReceiving = false;
      break;
    } else if (res < 0) {
      // error
      bIsReceiving = false;
      break;
    }

    PacketPtr packet = std::make_unique<uint8_t[]>(res);
    std::memcpy(packet.get(), messageBuffer.data(), res);
    recvQueue->Push(std::move(packet));
  }
  std::cout << "Receive thread ending.\n";
  eventDispatcher->Publish(QuitEvent{});
}

void ClientState::RegisterComponent() {
  // Register all component type inside typeArray to regsitry
  // powered by Lambda TMP Magic™
  using ComponentTypes =
      typeArray<AnimationComponent, AssemblingMachineComponent,
                BuildingComponent, BuildingPreviewComponent, CameraComponent,
                ChunkComponent, DebugRectComponent, InactiveComponent,
                InventoryComponent,InterpBufferComponent, InputStateComponent, MiningDrillComponent, MovableComponent,
                MovementComponent, NetPredictionComponent, LocalPlayerComponent,
                PlayerStateComponent, RefineryComponent, ResourceNodeComponent,
                SpriteComponent, TimerComponent, TimerExpiredTag,
                TransformComponent, TextComponent>;

  [reg = registry.get()]<std::size_t... Is>(std::index_sequence<Is...>) {
    ((reg->RegisterComponent<
         std::tuple_element_t<Is, typename ComponentTypes::typesTuple>>()),
     ...);
  }(std::make_index_sequence<ComponentTypes::size>{});
}

void ClientState::InitCoreSystem() {
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
  networkSystem = std::make_unique<ClientNetworkSystem>(systemContext);
  refinerySystem = std::make_unique<RefinerySystem>(systemContext);
  resourceNodeSystem = std::make_unique<ResourceNodeSystem>(systemContext);
  timerExpireSystem = std::make_unique<TimerExpireSystem>(systemContext);
  timerSystem = std::make_unique<TimerSystem>(systemContext);
  uiSystem = std::make_unique<UISystem>(systemContext);

  renderSystem =
      std::make_unique<RenderSystem>(systemContext, gRenderer, gFont);
}

void ClientState::Cleanup() {
  bIsReceiving = false;
  messageThread.join();
}

void ClientState::Update(float deltaTime) {
  if (bIsQuit) {
    if (!gEngine->IsChangeRequested())
      gEngine->ChangeState(std::make_unique<MainMenuState>());
    return;  // The state is now being destroyed, so we should not continue.
  }
  networkSystem->Update(deltaTime);

  // Process all pending commands.
  while (!commandQueue->IsEmpty()) {
    std::unique_ptr<Command> command = commandQueue->Dequeue();
    if (command) {
      command->Execute(registry.get(), eventDispatcher.get(), world.get());
    }
  }

  if (world->GetLocalPlayer() == INVALID_ENTITY) return;
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

  renderSystem->Update();
  uiSystem->Update();  // Display UI on very top
}