#ifndef CORE_TYPE_
#define CORE_TYPE_

#include <iostream>

struct Vec2f;

struct Vec2 {
  int x, y;
  constexpr Vec2(int _x = 0, int _y = 0) : x(_x), y(_y) {}
  constexpr Vec2(float _x, float _y)
      : x(static_cast<int>(_x)), y(static_cast<int>(_y)) {}

  constexpr Vec2 operator+(const Vec2& other) const {
    return {x + other.x, y + other.y};
  }

  constexpr Vec2 operator-(const Vec2& other) const {
    return {x - other.x, y - other.y};
  }
};

constexpr Vec2 operator*(const Vec2 a, const int b) {
  return {a.x * b, a.y * b};
}

constexpr Vec2 operator/(const Vec2 a, const int b) {
  if (b != 0)
    return {a.x / b, a.y / b};
  else
    return a;
}

struct Vec2f {
  float x, y;
  constexpr Vec2f(float _x = 0.f, float _y = 0.f) : x(_x), y(_y) {}

  constexpr Vec2f(const Vec2& other)
      : x(static_cast<float>(other.x)), y(static_cast<float>(other.y)) {}

  constexpr explicit operator Vec2() const {
    return {static_cast<int>(x), static_cast<int>(y)};
  }

  constexpr Vec2f operator+(const Vec2f& other) const {
    return {x + other.x, y + other.y};
  }

  constexpr Vec2f operator-(const Vec2f& other) const {
    return {x - other.x, y - other.y};
  }
};

constexpr Vec2f operator*(Vec2f a, float b) { return {a.x * b, a.y * b}; }

constexpr Vec2f operator/(Vec2f a, float b) {
  if (b != 0.f)
    return {a.x / b, a.y / b};
  else
    return a;
}

inline std::ostream& operator<<(std::ostream& out, const Vec2f& a) {
  out << "( " << a.x << ", " << a.y << " )";
  return out;
}

inline std::ostream& operator<<(std::ostream& out, const Vec2& a) {
  out << "( " << a.x << ", " << a.y << " )";
  return out;
}

#endif /* CORE_TYPE_ */
