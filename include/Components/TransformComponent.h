#ifndef COMPONENTS_TRANSFORMCOMPONENT_
#define COMPONENTS_TRANSFORMCOMPONENT_

#include "Core/Type.h"

struct TransformComponent {
  Vec2f position;
  Vec2f scale = {1.f, 1.f};
  float rotation = 0.f;  // Rotation in degrees
};

#endif /* COMPONENTS_TRANSFORMCOMPONENT_ */
