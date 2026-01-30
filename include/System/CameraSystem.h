#pragma once

#include "Core/Entity.h"
#include "Core/SystemContext.h"

class CameraSystem {
  Registry* registry;
  World* world;
  InputManager* inputManager;
  Entity cameraEntity;
  Entity localPlayer = Entity::Null();

 public:
  explicit CameraSystem(const SystemContext& context);
  ~CameraSystem();
  void InitCameraSystem();

  void Update(float deltaTime);

 private:
  void UpdateCameraFollow(float deltaTime);
  void UpdateCameraDrag(float deltaTime);
};
