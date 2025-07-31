#ifndef COMPONENTS_CAMERACOMPONENT_
#define COMPONENTS_CAMERACOMPONENT_

#include "Type.h"

struct CameraComponent {
  Vec2f position;           // Camera position in world coordinates
  Vec2f target;             // Target position to follow
  Vec2f offset;             // Manual offset from dragging
  float followSpeed;        // How fast camera follows target
  bool isFollowing;         // Whether camera should follow target
  bool isDragging;          // Whether camera is being dragged by mouse
  Vec2f dragStartPos;       // Mouse position when drag started
  Vec2f cameraStartPos;     // Camera position when drag started
  
  CameraComponent(Vec2f initialPos = {0.0f, 0.0f}, float speed = 10.f) 
    : position(initialPos), target(initialPos), offset{0.0f, 0.0f}, 
      followSpeed(speed), isFollowing(true), isDragging(false),
      dragStartPos{0.0f, 0.0f}, cameraStartPos{0.0f, 0.0f} {}
};

#endif /* COMPONENTS_CAMERACOMPONENT_ */