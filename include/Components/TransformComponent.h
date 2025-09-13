#ifndef COMPONENTS_TRANSFORMCOMPONENT_
#define COMPONENTS_TRANSFORMCOMPONENT_

#include "Core/Type.h"

struct TransformComponent {
  // TODO : make pos a local offset in int grid like chunk/tile index
  Vec2f position;
  Vec2f scale;
  float rotation;  // Rotation in degrees
  bool bIsDirty;
  constexpr TransformComponent(Vec2f position = {0.f, 0.f},
                               Vec2f scale = {1.f, 1.f}, float rotation = 0.f)
      : position(position), scale(scale), rotation(rotation), bIsDirty(false) {
        };
};

#endif /* COMPONENTS_TRANSFORMCOMPONENT_ */
