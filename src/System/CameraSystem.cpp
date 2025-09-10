#include "System/CameraSystem.h"

#include "Components/CameraComponent.h"
#include "Components/TransformComponent.h"
#include "Core/InputManager.h"
#include "Core/Registry.h"
#include "Core/World.h"

CameraSystem::CameraSystem(const SystemContext &context)
    : registry(context.registry),
      world(context.world),
      inputManager(context.inputManager) {
  InitCameraSystem();
}

void CameraSystem::InitCameraSystem() {
  cameraEntity = registry->CreateEntity();
  registry->EmplaceComponent<CameraComponent>(cameraEntity);
}

void CameraSystem::Update(float deltaTime) {
  playerEntity = world->GetPlayer();

  if (playerEntity == INVALID_ENTITY) return;

  UpdateCameraFollow(deltaTime);
  UpdateCameraDrag(deltaTime);
}

void CameraSystem::UpdateCameraFollow(float deltaTime) {
  auto &camera = registry->GetComponent<CameraComponent>(cameraEntity);

  // Update target position to player position
  if (registry->HasComponent<TransformComponent>(playerEntity)) {
    const auto &playerTransform =
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
  auto &camera = registry->GetComponent<CameraComponent>(cameraEntity);

  // Check if player is moving (disable drag if moving)
  bool playerIsMoving = false;
  playerIsMoving = abs(inputManager->GetXAxis()) > 0.1f ||
                   abs(inputManager->GetYAxis()) > 0.1f;

  // Start dragging
  if (inputManager->WasMouseButtonPressed(MouseButton::RIGHT) &&
      !playerIsMoving) {
    camera.isDragging = true;
    camera.isFollowing = false;
    camera.dragStartPos = Vec2f(inputManager->GetMousePosition());
    camera.cameraStartPos = camera.position;
  }

  if (inputManager->GetMouseWheelScroll() != 0) {
    camera.zoom = camera.zoom * ((inputManager->GetMouseWheelScroll()) * 0.1);
    // inputManager->mouseWheel = {0, 0};
  }

  // Continue dragging
  if (camera.isDragging &&
      inputManager->IsMouseButtonDown(MouseButton::RIGHT)) {
    Vec2f currentMousePos = inputManager->GetMousePosition();
    Vec2f mouseDelta =
        Vec2f(inputManager->GetMousePosition()) - camera.dragStartPos;

    // Update camera position (invert delta for natural dragging feel)
    camera.position = {camera.cameraStartPos.x - mouseDelta.x,
                       camera.cameraStartPos.y - mouseDelta.y};
  }

  // End dragging
  if (inputManager->WasMouseButtonReleased(MouseButton::RIGHT) ||
      playerIsMoving) {
    if (camera.isDragging) {
      // Store the offset for smooth transition back to following
      if (registry->HasComponent<TransformComponent>(playerEntity)) {
        const auto &playerTransform =
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

CameraSystem::~CameraSystem() = default;