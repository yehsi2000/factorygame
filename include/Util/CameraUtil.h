#ifndef UTIL_CAMERAUTIL_
#define UTIL_CAMERAUTIL_

#include "Core/Registry.h"
#include "Core/Type.h"

namespace util {

Vec2f GetCameraPosition(Registry *registry);

float GetCameraZoom(Registry *registry);

Vec2f WorldToScreen(Vec2f worldPos, Vec2f cameraPos, Vec2 screenSize,
                    float zoom = 1.0f);

Vec2f ScreenToWorld(Vec2f screenPos, Vec2f cameraPos, Vec2 screenSize,
                    float zoom = 1.0f);
} // namespace util

#endif /* UTIL_CAMERAUTIL_ */
