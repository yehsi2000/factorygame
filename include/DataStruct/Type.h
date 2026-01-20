#pragma once

#include <iostream>
#include <type_traits>

struct Vec2f;

/**
 * @brief A 2D vector of integers.
 * @details Used for representing coordinates and dimensions in discrete space,
 * such as tile indices and screen positions.
 */
struct Vec2 {
  int x, y;

  constexpr Vec2() = default;
  constexpr Vec2(int _x, int _y) : x(_x), y(_y) {}
  constexpr Vec2(float _x, float _y)
      : x(static_cast<int>(_x)), y(static_cast<int>(_y)) {}

  constexpr friend auto operator<=>(const Vec2&, const Vec2&) = default;
};

constexpr Vec2 operator+(Vec2 a, const Vec2& b) {
  a.x += b.x;
  a.y += b.y;
  return a;
}

constexpr Vec2 operator-(Vec2 a, const Vec2& b) {
  a.x -= b.x;
  a.y -= b.y;
  return a;
}

constexpr Vec2 operator*(Vec2 a, const int b) {
  a.x *= b;
  a.y *= b;
  return a;
}

constexpr Vec2 operator/(Vec2 a, const int b) {
  if (b != 0)
    return {a.x / b, a.y / b};
  else
    return a;
}

/**
 * @brief A 2D vector of floats.
 * @details Used for representing coordinates and velocities in continuous
 * space, such as world positions and entity movements.
 */
struct Vec2f {
  float x, y;

  constexpr Vec2f() = default;

  constexpr Vec2f(float _x, float _y) : x(_x), y(_y) {}

  constexpr Vec2f(const Vec2& other)
      : x(static_cast<float>(other.x)), y(static_cast<float>(other.y)) {}

  constexpr explicit operator Vec2() const {
    return {static_cast<int>(x), static_cast<int>(y)};
  }
  
  constexpr friend auto operator<=>(const Vec2f&, const Vec2f&) = default;
};

constexpr Vec2f operator+(Vec2f a, Vec2f b) {
  a.x += b.x;
  a.y += b.y;
  return a;
}

constexpr Vec2f operator-(Vec2f a, Vec2f b) {
  a.x -= b.x;
  a.y -= b.y;
  return a;
}

constexpr Vec2f operator*(Vec2f a, float b) {
  a.x *= b;
  a.y *= b;
  return a;
}

constexpr Vec2f operator/(Vec2f a, float b) {
  if (b != 0.f) {
    a.x /= b;
    a.y /= b;
  }

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

static_assert(std::is_standard_layout_v<Vec2f>);
static_assert(std::is_standard_layout_v<Vec2>);

static_assert(std::is_trivially_copyable_v<Vec2f>);
static_assert(std::is_trivially_copyable_v<Vec2>);

static_assert(std::is_trivially_default_constructible_v<Vec2f>);
static_assert(std::is_trivially_default_constructible_v<Vec2>);

static_assert(std::is_trivially_destructible_v<Vec2f>);
static_assert(std::is_trivially_destructible_v<Vec2>);
