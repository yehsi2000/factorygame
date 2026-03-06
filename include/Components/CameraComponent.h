#pragma once

#include "DataStruct/Type.h"

struct CameraComponent {
  Vec2f position;        // Camera position in world coordinates
  Vec2f target;          // Target position to follow
  Vec2f offset;          // Manual offset from dragging
  float followSpeed;     // How fast camera follows target
  bool isFollowing;      // Whether camera should follow target
  bool isDragging;       // Whether camera is being dragged by mouse
  Vec2f dragStartPos;    // Mouse position when drag started
  Vec2f cameraStartPos;  // Camera position when drag started
  float zoom;

  constexpr CameraComponent() = default;

  constexpr explicit CameraComponent(Vec2f initialPos, float speed = 10.f)
      : position(initialPos),
        target(initialPos),
        offset{0.0f, 0.0f},
        followSpeed(speed),
        isFollowing(true),
        isDragging(false),
        dragStartPos{0.0f, 0.0f},
        cameraStartPos{0.0f, 0.0f},
        zoom(1.0f) {}
};

#ifndef DISABLE_COMP_TYPECHECK
#include <type_traits>
static_assert(std::is_trivially_copyable_v<CameraComponent> == true);
static_assert(std::is_trivially_destructible_v<CameraComponent> == true);
#endif