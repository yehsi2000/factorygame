#ifndef SYSTEM_CAMERASYSTEM_
#define SYSTEM_CAMERASYSTEM_

#include "Core/SystemContext.h"
#include "Core/Entity.h"

class CameraSystem {
  Registry* registry;
  World* world;
  InputManager* inputManager;
  EntityID cameraEntity;
  EntityID localPlayer = INVALID_ENTITY;

 public:
  CameraSystem(const SystemContext& context);
  ~CameraSystem();
  void InitCameraSystem();

  void Update(float deltaTime);
  
 private:
  void UpdateCameraFollow(float deltaTime);
  void UpdateCameraDrag(float deltaTime);
};

#endif /* SYSTEM_CAMERASYSTEM_ */
