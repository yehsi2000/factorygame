#ifndef CORE_GENGINE_
#define CORE_GENGINE_

#include <memory>
#include <vector>

#include "Core/CommandQueue.h"
#include "Core/Entity.h"
#include "Core/EventDispatcher.h"
#include "Core/GameState.h"
#include "Core/Registry.h"
#include "Core/TimerManager.h"
#include "Core/World.h"
#include "SDL_ttf.h"
#include "imgui.h"

class AnimationSystem;
class AssemblingMachineSystem;
class CameraSystem;
class InputSystem;
class InteractionSystem;
class InventorySystem;
class MovementSystem;
class RefinerySystem;
class RenderSystem;
class ResourceNodeSystem;
class TimerExpireSystem;
class TimerSystem;
class UISystem;

/**
 * @brief Main Engine Class
 * 
 */
class GEngine {  
  EntityID player; ///< Local player ID
  SDL_Window* gWindow;
  SDL_Renderer* gRenderer;
  TTF_Font* gFont;

  std::unique_ptr<GameState> currentState; ///< Current GameState

  std::unique_ptr<Registry> registry;
  std::unique_ptr<TimerManager> timerManager;
  std::unique_ptr<EventDispatcher> dispatcher;
  std::unique_ptr<CommandQueue> commandQueue;
  std::unique_ptr<World> world;

  EventHandle GameEndHandle; ///< EventHandle subscribing GameEnd Event

  std::unique_ptr<AnimationSystem> animationSystem;
  std::unique_ptr<AssemblingMachineSystem> assemblingMachineSystem;
  std::unique_ptr<CameraSystem> cameraSystem;
  std::unique_ptr<InputSystem> inputSystem;
  std::unique_ptr<InventorySystem> inventorySystem;
  std::unique_ptr<MovementSystem> movementSystem;
  std::unique_ptr<RefinerySystem> refinerySystem;
  std::unique_ptr<RenderSystem> renderSystem;
  std::unique_ptr<ResourceNodeSystem> resourceNodeSystem;
  std::unique_ptr<TimerSystem> timerSystem;
  std::unique_ptr<TimerExpireSystem> timerExpireSystem;
  std::unique_ptr<InteractionSystem> interactionSystem;
  std::unique_ptr<UISystem> uiSystem;

  bool bIsRunning = true;

  GEngine(const GEngine&) = delete;
  GEngine& operator=(const GEngine&) = delete;
  GEngine(GEngine&&) = delete;
  GEngine& operator=(GEngine&&) = delete;
  void RegisterComponent();
  void InitCoreSystem();
  void GeneratePlayer();

 public:
 /**
  * @brief Construct a new GEngine object
  * @note initializes Registry, TimeManager, Eventdispatcher, CommandQueue, World, GameEndHandle.
  */
  GEngine(SDL_Window* window, SDL_Renderer* renderer, TTF_Font* font);
  ~GEngine();

  /**
   * @brief Change GameState to given state
   * 
   * @param newState new GameState to shift
   */
  void ChangeState(std::unique_ptr<GameState> newState);
  /**
   * @brief Process all pending command and update every system
   * 
   * @param deltaTime
   */
  void Update(float deltaTime);

  Vec2 GetScreenSize();

  inline EventDispatcher* GetDispatcher() { return dispatcher.get(); }
  inline CommandQueue* GetCommandQueue() { return commandQueue.get(); }
  inline EntityID GetPlayer() { return player; }
  inline Registry* GetRegistry() { return registry.get(); }
  inline TimerManager* GetTimerManager() { return timerManager.get(); }
  inline SDL_Renderer* GetRenderer() { return gRenderer; }
  inline SDL_Window* GetWindow() { return gWindow; }
  inline TTF_Font* GetFont() { return gFont; }
  inline World* GetWorld() { return world.get(); }
  inline bool IsRunning() const { return bIsRunning; }
};

#endif /* CORE_GENGINE_ */
