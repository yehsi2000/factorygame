#ifndef SYSTEM_MOVEMENTSYSTEM_
#define SYSTEM_MOVEMENTSYSTEM_

#include "Core/SystemContext.h"

class MovementSystem {
  Registry* registry;
  TimerManager* timerManager;
  InputManager* inputManager;
  EventDispatcher* eventDispatcher;
  World* world;
  bool bIsServer;

 public:
  MovementSystem(const SystemContext& context);
  ~MovementSystem();
  void Update(float deltaTime);

  private:
  void AddServerMoveIntent(float deltaTime);
  void ServerUpdate(float deltaTime);
  void ClientUpdate(float deltaTime);
};

#endif /* SYSTEM_MOVEMENTSYSTEM_ */
