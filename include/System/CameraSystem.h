#ifndef SYSTEM_CAMERASYSTEM_
#define SYSTEM_CAMERASYSTEM_

#include "Registry.h"
#include "Entity.h"
#include "Type.h"

class CameraSystem {
  Registry* registry;
  EntityID playerEntity;
  EntityID cameraEntity;
  
 public:
  CameraSystem(Registry* r, EntityID player);
  
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
  
  EntityID CreateCameraEntity();
};

#endif /* SYSTEM_CAMERASYSTEM_ */