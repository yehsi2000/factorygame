#ifndef UTIL_CAMERAUTIL_
#define UTIL_CAMERAUTIL_

#include "Core/Registry.h"
#include "Core/Type.h"

namespace util {

Vec2f GetCameraPosition(Registry* registry);

Vec2f WorldToScreen(Vec2f worldPos, Vec2f cameraPos, Vec2 screenSize);

Vec2f ScreenToWorld(Vec2f screenPos, Vec2f cameraPos, Vec2 screenSize);
}  // namespace util

#endif /* UTIL_CAMERAUTIL_ */
