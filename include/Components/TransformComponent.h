#pragma once

#include "DataStruct/Type.h"

struct TransformComponent {
  // TODO : make pos a local offset in int grid like chunk/tile index
  Vec2f position;
  Vec2f scale;
  float rotation;  // Rotation in degrees
  bool bIsDirty;

  constexpr TransformComponent() = default;

  constexpr TransformComponent(Vec2f position, Vec2f scale = {1.f, 1.f},
                               float rotation = 0.f)
      : position(position), scale(scale), rotation(rotation), bIsDirty(false) {
        };
};
