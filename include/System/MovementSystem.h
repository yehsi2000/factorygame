#pragma once

#include "Core/SystemContext.h"

class MovementSystem {
  Registry* registry;
  TimerManager* timerManager;
  InputManager* inputManager;
  EventDispatcher* eventDispatcher;
  World* world;
  bool bIsServer;

  ThreadSafeQueue<MoveAppliedPtr>* pendingMoves;

 public:
  explicit MovementSystem(const SystemContext& context);
  ~MovementSystem();
  void Update(float deltaTime);

 private:
  void ServerUpdate(float deltaTime);
  void ClientUpdate(float deltaTime);
};
