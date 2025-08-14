#include "System/CameraSystem.h"

#include "Components/CameraComponent.h"
#include "Components/TransformComponent.h"
#include "Core/GEngine.h"
#include "Core/Registry.h"

CameraSystem::CameraSystem(GEngine* e) : engine(e) {
  registry = engine->GetRegistry();
  assert(registry && "Fail to initialize CameraSystem : Invalid registry");
}

void CameraSystem::InitCameraSystem() {
  EntityID camera = registry->CreateEntity();
  registry->EmplaceComponent<CameraComponent>(camera);
  cameraEntity = camera;
}

void CameraSystem::Update(float deltaTime) {
  playerEntity = engine->GetPlayer();
  UpdateCameraFollow(deltaTime);
  UpdateCameraDrag(deltaTime);
}

Vec2f CameraSystem::GetCameraPosition() const {
  if (registry->HasComponent<CameraComponent>(cameraEntity)) {
    const auto& camera = registry->GetComponent<CameraComponent>(cameraEntity);
    return camera.position;
  }
  return {0.0f, 0.0f};
}

Vec2f CameraSystem::WorldToScreen(Vec2f worldPos, int screenWidth,
                                  int screenHeight) const {
  Vec2f cameraPos = GetCameraPosition();
  return {worldPos.x - cameraPos.x + screenWidth / 2.0f,
          worldPos.y - cameraPos.y + screenHeight / 2.0f};
}

Vec2f CameraSystem::ScreenToWorld(Vec2f screenPos, int screenWidth,
                                  int screenHeight) const {
  Vec2f cameraPos = GetCameraPosition();
  return {screenPos.x + cameraPos.x - screenWidth / 2.0f,
          screenPos.y + cameraPos.y - screenHeight / 2.0f};
}

void CameraSystem::UpdateCameraFollow(float deltaTime) {
  if (playerEntity != INVALID_ENTITY &&
      !registry->HasComponent<CameraComponent>(cameraEntity))
    return;

  auto& camera = registry->GetComponent<CameraComponent>(cameraEntity);

  // Update target position to player position
  if (registry->HasComponent<TransformComponent>(playerEntity)) {
    const auto& playerTransform =
        registry->GetComponent<TransformComponent>(playerEntity);
    camera.target = playerTransform.position;
  }

  // Only follow if not dragging and following is enabled
  if (camera.isFollowing && !camera.isDragging) {
    // Smooth camera movement towards target
    Vec2f targetPos = camera.target + camera.offset;
    Vec2f direction = {targetPos.x - camera.position.x,
                       targetPos.y - camera.position.y};

    float distance =
        sqrtf(direction.x * direction.x + direction.y * direction.y);
    if (distance > 0.1f) {
      direction.x /= distance;
      direction.y /= distance;

      camera.position.x +=
          direction.x * camera.followSpeed * deltaTime * distance;
      camera.position.y +=
          direction.y * camera.followSpeed * deltaTime * distance;
    } else {
      camera.position = targetPos;
    }
  }
}

void CameraSystem::UpdateCameraDrag(float deltaTime) {
  if (playerEntity != INVALID_ENTITY &&
      !registry->HasComponent<CameraComponent>(cameraEntity))
    return;

  auto& camera = registry->GetComponent<CameraComponent>(cameraEntity);
  const auto& input = registry->GetInputState();

  // Check if player is moving (disable drag if moving)
  bool playerIsMoving = false;
  const auto& inputState = registry->GetInputState();
  playerIsMoving =
      (abs(inputState.xAxis) > 0.1f || abs(inputState.yAxis) > 0.1f);

  // Start dragging
  if (input.rightMousePressed && !playerIsMoving) {
    camera.isDragging = true;
    camera.isFollowing = false;
    camera.dragStartPos = {(float)input.mouseX, (float)input.mouseY};
    camera.cameraStartPos = camera.position;
  }

  // Continue dragging
  if (camera.isDragging && input.rightMouseDown) {
    Vec2f currentMousePos = {(float)input.mouseX, (float)input.mouseY};
    Vec2f mouseDelta = {currentMousePos.x - camera.dragStartPos.x,
                        currentMousePos.y - camera.dragStartPos.y};

    // Update camera position (invert delta for natural dragging feel)
    camera.position = {camera.cameraStartPos.x - mouseDelta.x,
                       camera.cameraStartPos.y - mouseDelta.y};
  }

  // End dragging
  if (input.rightMouseReleased || playerIsMoving) {
    if (camera.isDragging) {
      // Store the offset for smooth transition back to following
      if (registry->HasComponent<TransformComponent>(playerEntity)) {
        const auto& playerTransform =
            registry->GetComponent<TransformComponent>(playerEntity);
        camera.offset = {camera.position.x - playerTransform.position.x,
                         camera.position.y - playerTransform.position.y};
      }

      camera.isDragging = false;
      // Re-enable following when player starts moving or after a delay
      if (playerIsMoving) {
        camera.isFollowing = true;
        // Gradually reduce offset when player moves
        camera.offset.x *= 0.95f;
        camera.offset.y *= 0.95f;
        if (abs(camera.offset.x) < 0.1f && abs(camera.offset.y) < 0.1f) {
          camera.offset = {0.0f, 0.0f};
        }
      }
    }
  }

  // Auto re-enable following after some time of inactivity
  static float inactiveTime = 0.0f;
  if (!camera.isDragging && !camera.isFollowing) {
    inactiveTime += deltaTime;
    if (inactiveTime > 3.0f) {  // 3 seconds of inactivity
      camera.isFollowing = true;
      camera.offset = {0.0f, 0.0f};
      inactiveTime = 0.0f;
    }
  } else {
    inactiveTime = 0.0f;
  }
}
