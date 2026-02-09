#include "GameState/ClientState.h"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <optional>
#include <utility>

#include "Core/CommandQueue.h"
#include "Core/EntityFactory.h"
#include "Core/Event.h"
#include "Core/EventDispatcher.h"
#include "Core/GEngine.h"
#include "Core/Packet.h"
#include "Core/Registry.h"
#include "Core/Socket.h"
#include "Core/TimerManager.h"
#include "Core/World.h"
#include "DataStruct/ThreadSafeQueue.h"
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
#include "Util/PacketUtil.h"


ClientState::ClientState() : gEngine(nullptr), isQuit(false) {}
ClientState::~ClientState() = default;

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
  systemContext.isServer = false;
  InitCoreSystem();

  GameEndEventHandle =
      eventDispatcher->Subscribe<QuitEvent>([this](const QuitEvent& e) {
        isQuit = true;  // Signal the main thread to quit
      });

  // TODO : move message buffer and receiving thread to network system
  messageBuffer = std::vector<uint8_t>(MAX_BUFFER);

  isReceiving = isSending = true;
  recvThread = std::thread([this] { SocketReceiveWorker(); });
  sendThread = std::thread([this] { SocketSendWorker(); });
  networkSystem->Init(u8"Client");
}

bool ClientState::TryConnect(std::string ip) {
  connectionSocket = std::make_unique<Socket>();
  connectionSocket->Init();

  int res = connectionSocket->Connect(std::move(ip), 27015);
  // TODO : send duplicate name check packet and return if duplicate name exists

  if (res == 0) return false;
  return true;
}

void ClientState::SocketReceiveWorker() {
  while (isReceiving) {
    int res =
        connectionSocket->Receive(messageBuffer.data(), messageBuffer.size());

    if (res == 0) {
      // Connection closed
      std::cout << "Connection closed by server.\n";
      isReceiving = false;
      isSending = false;
      break;
    } else if (res < 0) {
      // error
      isReceiving = false;
      isSending = false;
      break;
    }

    PacketPtr packet = std::make_unique<Packet>(res);
    std::memcpy(packet.get()->data(), messageBuffer.data(), res);
    recvQueue->Push(std::move(packet));
  }
  std::cout << "Receive thread ending.\n";
  sendQueue->Shutdown();
  eventDispatcher->Publish(QuitEvent{});
}

void ClientState::SocketSendWorker() {
  try {
    while (isSending) {
      std::optional<PacketPtr> packet = sendQueue->WaitAndPop();
      if (!packet.has_value()) return;
      const uint8_t* rp = packet.value()->data();
      std::size_t packetSize;
      PACKET packetId;
      util::GetHeader(rp, packetId, packetSize);
      connectionSocket->Send(packet.value()->data(), packetSize);
    }
  } catch (const std::runtime_error& e) {
    std::cout << "Send thread ending due to queue shutdown: " << e.what()
              << std::endl;
  }
  std::cout << "Send thread ending.\n";
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
  isSending = false;
  isReceiving = false;
  if (sendThread.joinable()) sendThread.join();
  if (recvThread.joinable()) recvThread.join();
}

void ClientState::Update(float deltaTime) {
  if (isQuit) {
    if (!gEngine->IsChangeRequested())
      gEngine->ChangeState(std::make_unique<MainMenuState>());
    return;  // The state is now being destroyed, so we should not continue.
  }
  networkSystem->Update(deltaTime);

  if (world->GetLocalPlayer() == Entity::Null()) return;
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