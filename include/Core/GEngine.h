#ifndef CORE_GENGINE_
#define CORE_GENGINE_

#include <memory>
#include <vector>

#include "Core/CommandQueue.h"
#include "Core/Entity.h"
#include "Core/Event.h"
#include "Core/EventDispatcher.h"
#include "Core/GameState.h"
#include "Core/Item.h"
#include "Core/Registry.h"
#include "Core/TimerManager.h"
#include "Core/World.h"
#include "SDL.h"
#include "SDL_ttf.h"
#include "System/AnimationSystem.h"
#include "System/CameraSystem.h"
#include "System/InputSystem.h"
#include "System/InteractionSystem.h"
#include "System/InventorySystem.h"
#include "System/MovementSystem.h"
#include "System/RefinerySystem.h"
#include "System/RenderSystem.h"
#include "System/ResourceNodeSystem.h"
#include "System/TimerExpireSystem.h"
#include "System/TimerSystem.h"

class GEngine {
  EntityID player;

  std::vector<EntityID> entities;
  std::unique_ptr<EventDispatcher> dispatcher;
  std::unique_ptr<CommandQueue> commandQueue;
  std::unique_ptr<GameState> currentState;
  std::unique_ptr<Registry> registry;
  std::unique_ptr<TimerManager> timerManager;

  std::shared_ptr<ItemDatabase> itemDatabase;

  std::unique_ptr<AnimationSystem> animationSystem;
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

  SDL_Window* gWindow;
  SDL_Renderer* gRenderer;
  TTF_Font* gFont;

  World* world;

  bool bIsRunning = true;

  EntityID nextID = 1;

  GEngine(const GEngine&) = delete;
  GEngine& operator=(const GEngine&) = delete;
  GEngine(GEngine&&) = delete;
  GEngine& operator=(GEngine&&) = delete;
  void RegisterComponent();
  void InitCoreSystem();
  void GeneratePlayer();
  std::unique_ptr<EventHandle> GameEndHandle;

 public:
  GEngine(SDL_Window* window, SDL_Renderer* renderer, TTF_Font* font);

  void ChangeState(std::unique_ptr<GameState> newState);
  void Update(float deltaTime);

  inline EventDispatcher* GetDispatcher() { return dispatcher.get(); }
  inline CommandQueue* GetCommandQueue() { return commandQueue.get(); }
  inline EntityID GetPlayer() { return player; }
  inline Registry* GetRegistry() { return registry.get(); }
  inline TimerManager* GetTimerManager() { return timerManager.get(); }
  inline SDL_Renderer* GetRenderer() { return gRenderer; }
  inline SDL_Window* GetWindow() { return gWindow; }
  inline TTF_Font* GetFont() { return gFont; }
  inline World* GetWorld() { return world; }
  inline bool IsRunning() const { return bIsRunning; }
};

#endif /* CORE_GENGINE_ */
