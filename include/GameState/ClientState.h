#ifndef GAMESTATE_CLIENTSTATE_
#define GAMESTATE_CLIENTSTATE_

#include <memory>
#include <tuple>
#include <thread>
#include <vector>

#include "Core/Entity.h"
#include "Core/EventDispatcher.h"
#include "Core/SystemContext.h"
#include "GameState/IGameState.h"
#include "Core/ThreadSafeQueue.h"
#include "Core/Packet.h"
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
class ClientState : public IGameState {
  SDL_Window *gWindow;
  SDL_Renderer *gRenderer;
  TTF_Font *gFont;
  AssetManager *assetManager;
  WorldAssetManager *worldAssetManager;
  GEngine* gEngine;

  std::unique_ptr<Registry> registry;
  std::unique_ptr<TimerManager> timerManager;
  std::unique_ptr<EventDispatcher> eventDispatcher;
  std::unique_ptr<CommandQueue> commandQueue;
  std::unique_ptr<EntityFactory> entityFactory;
  std::unique_ptr<World> world;

  SystemContext systemContext;
  std::unique_ptr<EventHandle> GameEndEventHandle;
  EntityID player;

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
  
  std::unique_ptr<Socket> connectionSocket;
  std::unique_ptr<ThreadSafeQueue<PacketPtr>> packetQueue;
  std::unique_ptr<ThreadSafeQueue<SendRequest>> sendQueue;

  uintptr_t clientID;
  std::vector<char> messageBuffer;
  std::thread messageThread;
  bool bIsReceiving;
  
  Vec2 screenSize;

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
