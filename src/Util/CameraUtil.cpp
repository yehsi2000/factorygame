#include "Util/CameraUtil.h"
#include "Components/CameraComponent.h"
#include "Core/Type.h"

namespace util {

Vec2f GetCameraPosition(Registry *registry) {
  // Find the first entity with a CameraComponent
  auto cameraView = registry->view<CameraComponent>();

  for (EntityID entity : cameraView) {
    const auto &camera = registry->GetComponent<CameraComponent>(entity);
    return camera.position;
  }

  // Default camera position if no camera found
  return {0.0f, 0.0f};
}

float GetCameraZoom(Registry *registry) {
  // Find the first entity with a CameraComponent
  auto cameraView = registry->view<CameraComponent>();

  for (EntityID entity : cameraView) {
    const auto &camera = registry->GetComponent<CameraComponent>(entity);
    return camera.zoom;
  }

  // Default camera position if no camera found
  return 1.0f;
}

Vec2f WorldToScreen(Vec2f worldPos, Vec2f cameraPos, Vec2 screenSize,
                    float zoom) {
  return {(worldPos.x - cameraPos.x) * zoom + screenSize.x / 2.0f,
          (worldPos.y - cameraPos.y) * zoom + screenSize.y / 2.0f};
}

Vec2f ScreenToWorld(Vec2f screenPos, Vec2f cameraPos, Vec2 screenSize,
                    float zoom) {
  return {(screenPos.x - screenSize.x / 2.0f) / zoom + cameraPos.x,
          (screenPos.y - screenSize.y / 2.0f) / zoom + cameraPos.y};
}

} // namespace util