#ifndef CORE_TYPE_
#define CORE_TYPE_

struct Vec2 {
  int x, y;
};

struct Vec2f {
  float x, y;

  Vec2f operator+(const Vec2f& other) const {
    return {x + other.x, y + other.y};
  }

  Vec2f operator-(const Vec2f& other) const {
    return {x - other.x, y - other.y};
  }
};

#endif /* CORE_TYPE_ */
