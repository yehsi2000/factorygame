#ifndef SYSTEM_CAMERASYSTEM_
#define SYSTEM_CAMERASYSTEM_

#include "Core/Entity.h"

class GEngine;
class Registry;

class CameraSystem {
  GEngine* engine;
  Registry* registry;
  EntityID cameraEntity;
  EntityID playerEntity = INVALID_ENTITY;

 public:
  CameraSystem(GEngine* e);
  void InitCameraSystem();

  void Update(float deltaTime);
  
 private:
  void UpdateCameraFollow(float deltaTime);
  void UpdateCameraDrag(float deltaTime);
};

#endif /* SYSTEM_CAMERASYSTEM_ */
