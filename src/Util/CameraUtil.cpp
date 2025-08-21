#include "Components/CameraComponent.h"
#include "Util/CameraUtil.h"
#include "Core/Type.h"

namespace util {

Vec2f GetCameraPosition(Registry* registry) {
  // Find the first entity with a CameraComponent
  auto cameraView = registry->view<CameraComponent>();

  for (EntityID entity : cameraView) {
    const auto& camera = registry->GetComponent<CameraComponent>(entity);
    return camera.position;
  }

  // Default camera position if no camera found
  return {0.0f, 0.0f};
}

Vec2f WorldToScreen(Vec2f worldPos, Vec2f cameraPos, Vec2 screenSize) {
  return {worldPos.x - cameraPos.x + screenSize.x / 2.0f,
          worldPos.y - cameraPos.y + screenSize.y / 2.0f};
}

Vec2f ScreenToWorld(Vec2f screenPos, Vec2f cameraPos, Vec2 screenSize) {
  return {screenPos.x + cameraPos.x - screenSize.x / 2.0f,
          screenPos.y + cameraPos.y - screenSize.y / 2.0f};
}

}  // namespace util