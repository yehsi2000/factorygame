#ifndef GAMESTATE_PLAYSTATE_
#define GAMESTATE_PLAYSTATE_

#include <memory>
#include <tuple>
#include <vector>

#include "Core/Entity.h"
#include "Core/EventDispatcher.h"
#include "Core/SystemContext.h"
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

class AnimationSystem;
class AssemblingMachineSystem;
class CameraSystem;
class InputSystem;
class InteractionSystem;
class ItemDragSystem;
class InventorySystem;
class MiningDrillSystem;
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
class PlayState : public IGameState {
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
  std::unique_ptr<RefinerySystem> refinerySystem;
  std::unique_ptr<RenderSystem> renderSystem;
  std::unique_ptr<ResourceNodeSystem> resourceNodeSystem;
  std::unique_ptr<TimerSystem> timerSystem;
  std::unique_ptr<TimerExpireSystem> timerExpireSystem;
  std::unique_ptr<InteractionSystem> interactionSystem;
  std::unique_ptr<UISystem> uiSystem;

  Vec2 screenSize;

 public:
  PlayState();
  virtual void Init(GEngine *engine) override;
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
  void RegisterComponent();
  void InitCoreSystem();
};

#endif /* GAMESTATE_PLAYSTATE_ */
