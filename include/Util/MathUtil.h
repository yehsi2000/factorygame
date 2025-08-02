#ifndef UTIL_MATHUTIL_
#define UTIL_MATHUTIL_

#include "Core/Type.h"
#include "cmath"

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

double dist(Vec2f a, Vec2f b) {
  double dx = static_cast<double>(a.x - b.x);
  double dy = static_cast<double>(a.y - b.y);
  return static_cast<double>(std::sqrt(dx * dx + dy * dy));
}

double dist(Vec2 a, Vec2 b) {
  return std::sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}

}  // namespace util

#endif /* UTIL_MATHUTIL_ */
