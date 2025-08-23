#ifndef COMPONENTS_TRANSFORMCOMPONENT_
#define COMPONENTS_TRANSFORMCOMPONENT_

#include "Core/Type.h"

struct TransformComponent {
  Vec2f position;
  Vec2f scale;
  float rotation;  // Rotation in degrees
  constexpr TransformComponent(Vec2f position = {0.f, 0.f},
                               Vec2f scale = {1.f, 1.f}, float rotation = 0.f)
      : position(position), scale(scale), rotation(rotation) {};
};

#endif /* COMPONENTS_TRANSFORMCOMPONENT_ */
