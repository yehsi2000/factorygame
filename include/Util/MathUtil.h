#ifndef UTIL_MATHUTIL_
#define UTIL_MATHUTIL_

#include "Core/Type.h"

namespace util {

template <typename T>
T map_range(T value, T start, T end, T mapped_start, T mapped_end) {
  if (start == end) return mapped_start;

  double proportion = static_cast<double>(value - start) / (end - start);

  return static_cast<T>(mapped_start +
                        proportion * (mapped_end - mapped_start));
}

template <typename T>
T clamp(T value, T min, T max) {
  if (value < min) return min;
  if (value > max) return max;
  return value;
}

template <typename T>
T Lerp(T a, T b, float t) {
  return a + (b - a) * t;
}

double dist(Vec2f a, Vec2f b);

double dist(Vec2 a, Vec2 b);

double dist(Vec2f d);

double dist(Vec2 d);

Vec2f WorldToScreen(Vec2f worldPos, Vec2f cameraPos, int screenWidth,
                    int screenHeight);

Vec2f ScreenToWorld(Vec2f screenPos, Vec2f cameraPos, int screenWidth,
                    int screenHeight);

}  // namespace util

#endif /* UTIL_MATHUTIL_ */
