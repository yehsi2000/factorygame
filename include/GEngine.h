#ifndef GENGINE_
#define GENGINE_

#include <memory>
#include <vector>

#include "Entity.h"
#include "Event.h"
#include "GameState.h"
#include "Item.h"
#include "Registry.h"
#include "SDL.h"
#include "System/AnimationSystem.h"
#include "System/InventorySystem.h"
#include "System/MovementSystem.h"
#include "System/RefinerySystem.h"
#include "System/RenderSystem.h"
#include "System/ResourceNodeSystem.h"
#include "System/TimerSystem.h"

class GEngine {
  EntityID player;

  std::vector<EntityID> entities;
  std::unique_ptr<EventDispatcher> dispatcher;
  std::unique_ptr<GameState> currentState;
  std::unique_ptr<Registry> registry;

  std::shared_ptr<ItemDatabase> itemDatabase;
  
  std::unique_ptr<AnimationSystem> animationSystem;
  std::unique_ptr<InventorySystem> inventorySystem;
  std::unique_ptr<MovementSystem> movementSystem;
  std::unique_ptr<RefinerySystem> refinerySystem;
  std::unique_ptr<RenderSystem> renderSystem;
  std::unique_ptr<ResourceNodeSystem> resourceNodeSystem;
  std::unique_ptr<TimerSystem> timerSystem;

  SDL_Window* gWindow;
  SDL_Renderer* gRenderer;

  EntityID nextID = 1;

  GEngine(const GEngine&) = delete;
  GEngine& operator=(const GEngine&) = delete;
  GEngine(GEngine&&) = delete;
  GEngine& operator=(GEngine&&) = delete;
  void RegisterComponent();
  void InitCoreSystem();
  void GeneratePlayer();

 public:
  GEngine(SDL_Window* window, SDL_Renderer* renderer);

  void ChangeState(std::unique_ptr<GameState> newState);
  void Update(float deltaTime);

  inline EventDispatcher* GetDispatcher() { return dispatcher.get(); }
  inline EntityID GetPlayer() { return player; }
  inline Registry* GetRegistry() { return registry.get(); }
};

#endif /* GENGINE_ */
