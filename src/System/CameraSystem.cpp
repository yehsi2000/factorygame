#include "System/CameraSystem.h"

#include "Components/CameraComponent.h"
#include "Components/TransformComponent.h"
#include "Core/InputPoller.h"
#include "Core/Registry.h"
#include "Core/World.h"

CameraSystem::CameraSystem(const SystemContext &context)
    : registry(context.registry),
      world(context.world),
      inputPoller(context.inputPoller) {
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
  playerIsMoving = abs(inputPoller->GetXAxis()) > 0.1f ||
                   abs(inputPoller->GetYAxis()) > 0.1f;

  // Start dragging
  if (inputPoller->WasMouseButtonPressed(InputPoller::Mouse::RIGHT) &&
      !playerIsMoving) {
    camera.isDragging = true;
    camera.isFollowing = false;
    camera.dragStartPos = Vec2f(inputPoller->GetMousePositon());
    camera.cameraStartPos = camera.position;
  }

  if (inputPoller->GetScrollAmount() != 0) {
    camera.zoom = camera.zoom * ((inputPoller->GetScrollAmount()) * 0.1);
    // inputPoller->mouseWheel = {0, 0};
  }

  // Continue dragging
  if (camera.isDragging &&
      inputPoller->IsMouseButtonDown(InputPoller::Mouse::RIGHT)) {
    Vec2f currentMousePos = inputPoller->GetMousePositon();
    Vec2f mouseDelta =
        Vec2f(inputPoller->GetMousePositon()) - camera.dragStartPos;

    // Update camera position (invert delta for natural dragging feel)
    camera.position = {camera.cameraStartPos.x - mouseDelta.x,
                       camera.cameraStartPos.y - mouseDelta.y};
  }

  // End dragging
  if (inputPoller->WasMouseButtonReleased(InputPoller::Mouse::RIGHT) ||
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