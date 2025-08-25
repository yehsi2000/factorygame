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
class MiningDrillSystem;
class MovementSystem;
class RefinerySystem;
class RenderSystem;
class ResourceNodeSystem;
class TimerExpireSystem;
class TimerSystem;
class UISystem;

/**
 * @brief The main engine class that drives the game.
 * @details Manages the game loop, owns all core systems and services,
 *          and orchestrates the overall game state.
 */
class GEngine {  
  
  SDL_Window* gWindow;
  SDL_Renderer* gRenderer;
  TTF_Font* gFont;

  std::unique_ptr<Registry> registry;
  std::unique_ptr<TimerManager> timerManager;
  std::unique_ptr<EventDispatcher> dispatcher;
  std::unique_ptr<CommandQueue> commandQueue;
  std::unique_ptr<World> world;

  EventHandle GameEndHandle; ///< EventHandle subscribing GameEnd Event

  EntityID player; ///< Local player ID
  std::unique_ptr<GameState> currentState; ///< Current GameState

  std::unique_ptr<AnimationSystem> animationSystem;
  std::unique_ptr<AssemblingMachineSystem> assemblingMachineSystem;
  std::unique_ptr<CameraSystem> cameraSystem;
  std::unique_ptr<InputSystem> inputSystem;
  std::unique_ptr<InventorySystem> inventorySystem;
  std::unique_ptr<MiningDrillSystem> miningDrillSystem;
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
   * @brief Constructs the GEngine.
   * @note Initializes all core services and systems.
   */
  GEngine(SDL_Window* window, SDL_Renderer* renderer, TTF_Font* font);
  ~GEngine();

  /**
   * @brief Changes the current game state (e.g., from menu to in-game).
   * @param newState The new GameState to activate.
   */
  void ChangeState(std::unique_ptr<GameState> newState);
  
  /**
   * @brief Executes a single frame of the game loop.
   * @details Processes all pending commands and then updates all active game systems.
   * @param deltaTime The time elapsed since the last frame.
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
