#ifndef GAMESTATE_CLIENTSTATE_
#define GAMESTATE_CLIENTSTATE_

#include <cstdint>
#include <cstdio>
#include <memory>
#include <thread>
#include <tuple>
#include <vector>

#include "Core/Entity.h"
#include "Core/EventDispatcher.h"
#include "Core/Packet.h"
#include "Core/SystemContext.h"
#include "Core/ThreadSafeQueue.h"
#include "GameState/IGameState.h"
#include "SDL_ttf.h"
#include "imgui.h"

class GEngine;
class CommandQueue;
class AssetManager;
class EntityFactory;
class IGameState;
class Registry;
class SystemContext;
class TimerManager;
class World;
class WorldAssetManager;
class Socket;

class AnimationSystem;
class AssemblingMachineSystem;
class CameraSystem;
class InputSystem;
class InteractionSystem;
class ItemDragSystem;
class InventorySystem;
class MiningDrillSystem;
class ClientNetworkSystem;
class MovementSystem;
class RenderSystem;
class RefinerySystem;
class ResourceNodeSystem;
class TimerExpireSystem;
class TimerSystem;
class UISystem;

/**
 * @brief Represents the primary gameplay state.
 */

constexpr std::size_t MAX_BUFFER = 1024;

class ClientState : public IGameState {
  GEngine *gEngine;
  SDL_Window *gWindow;
  SDL_Renderer *gRenderer;
  TTF_Font *gFont;
  AssetManager *assetManager;
  WorldAssetManager *worldAssetManager;

  std::unique_ptr<Registry> registry;
  std::unique_ptr<TimerManager> timerManager;
  std::unique_ptr<EventDispatcher> eventDispatcher;
  std::unique_ptr<CommandQueue> commandQueue;
  std::unique_ptr<EntityFactory> entityFactory;
  std::unique_ptr<World> world;
  std::unique_ptr<Socket> connectionSocket;

  std::unique_ptr<ThreadSafeQueue<PacketPtr>> recvQueue;
  std::unique_ptr<ThreadSafeQueue<PacketPtr>> sendQueue;

  std::unordered_map<clientid_t, std::string> clientNameMap;

  SystemContext systemContext;
  std::unique_ptr<EventHandle> GameEndEventHandle;
  EntityID player;

  std::vector<uint8_t> messageBuffer;
  std::thread messageThread;
  std::size_t clientID;
  bool bIsReceiving;
  bool bIsQuit;

  std::unique_ptr<AnimationSystem> animationSystem;
  std::unique_ptr<AssemblingMachineSystem> assemblingMachineSystem;
  std::unique_ptr<CameraSystem> cameraSystem;
  std::unique_ptr<InputSystem> inputSystem;
  std::unique_ptr<InventorySystem> inventorySystem;
  std::unique_ptr<ItemDragSystem> itemDragSystem;
  std::unique_ptr<MiningDrillSystem> miningDrillSystem;
  std::unique_ptr<MovementSystem> movementSystem;
  std::unique_ptr<ClientNetworkSystem> networkSystem;
  std::unique_ptr<RefinerySystem> refinerySystem;
  std::unique_ptr<RenderSystem> renderSystem;
  std::unique_ptr<ResourceNodeSystem> resourceNodeSystem;
  std::unique_ptr<TimerSystem> timerSystem;
  std::unique_ptr<TimerExpireSystem> timerExpireSystem;
  std::unique_ptr<InteractionSystem> interactionSystem;
  std::unique_ptr<UISystem> uiSystem;

 public:
  ClientState();
  virtual void Init(GEngine *engine) override;
  bool TryConnect();
  virtual void Cleanup() override;
  virtual void Update(float deltaTime) override;

  /**
   * @brief template struct for register component types
   *
   * @tparam Types All Component Types
   */
  template <typename... Types>
  struct typeArray {
    using typesTuple = std::tuple<Types...>;
    static constexpr std::size_t size = sizeof...(Types);
  };

 private:
  void SocketReceiveWorker();
  void RegisterComponent();
  void InitCoreSystem();
};

#endif/* GAMESTATE_CLIENTSTATE_ */
