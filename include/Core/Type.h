#ifndef CORE_TYPE_
#define CORE_TYPE_

struct Vec2 {
  int x, y;
  Vec2() : x(0), y(0) {}
  Vec2(int _x, int _y) : x(_x), y(_y) {}
  Vec2(float _x, float _y) : x(static_cast<int>(_x)), y(static_cast<int>(_y)) {}

  Vec2 operator+(const Vec2& other) const { return {x + other.x, y + other.y}; }

  Vec2 operator-(const Vec2& other) const { return {x - other.x, y - other.y}; }
};

struct Vec2f {
  float x, y;

  Vec2f() : x(0.f), y(0.f) {}
  Vec2f(float _x, float _y) : x(_x), y(_y) {}
  Vec2f(int _x, int _y)
      : x(static_cast<float>(_x)), y(static_cast<float>(_y)) {};

  Vec2f operator+(const Vec2f& other) const {
    return {x + other.x, y + other.y};
  }

  Vec2f operator-(const Vec2f& other) const {
    return {x - other.x, y - other.y};
  }
};

#endif /* CORE_TYPE_ */
