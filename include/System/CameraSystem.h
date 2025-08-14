#ifndef SYSTEM_CAMERASYSTEM_
#define SYSTEM_CAMERASYSTEM_

#include "Core/Entity.h"
#include "Core/Type.h"

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

  // Get camera position for rendering
  Vec2f GetCameraPosition() const;

  // Convert world coordinates to screen coordinates
  Vec2f WorldToScreen(Vec2f worldPos, int screenWidth, int screenHeight) const;

  // Convert screen coordinates to world coordinates
  Vec2f ScreenToWorld(Vec2f screenPos, int screenWidth, int screenHeight) const;

 private:
  void UpdateCameraFollow(float deltaTime);
  void UpdateCameraDrag(float deltaTime);
};

#endif /* SYSTEM_CAMERASYSTEM_ */
