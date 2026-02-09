#include "Util/MathUtil.h"

#include <cmath>

#include "DataStruct/Type.h"


namespace util {

double dist(Vec2f a, Vec2f b) {
  auto dx = static_cast<double>(a.x - b.x);
  auto dy = static_cast<double>(a.y - b.y);
  return static_cast<double>(std::sqrt(dx * dx + dy * dy));
}

double dist(Vec2 a, Vec2 b) {
  return std::sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}

double dist(Vec2f d) {
  return static_cast<double>(std::sqrt(d.x * d.x + d.y * d.y));
}

double dist(Vec2 d) {
  return static_cast<double>(std::sqrt(d.x * d.x + d.y * d.y));
}

}  // namespace util