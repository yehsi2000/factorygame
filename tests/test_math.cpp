#include "Util/MathUtil.h"
#include <SDL.h>
#include <cmath>
#include <iostream>

bool test_map_range() {
  // Test map_range function
  auto result = util::map_range(5, 0, 10, 0, 100);
  if (result != 50) {
    std::cerr << "map_range test failed: expected 50, got " << result
              << std::endl;
    return false;
  }

  auto result2 = util::map_range(0.5f, 0.0f, 1.0f, 100.0f, 200.0f);
  if (std::abs(result2 - 150.0f) > 0.001f) {
    std::cerr << "map_range test 2 failed: expected 150, got " << result2
              << std::endl;
    return false;
  }

  return true;
}

bool test_clamp() {
  // Test clamp function
  if (util::clamp(5, 0, 10) != 5) {
    std::cerr << "clamp test 1 failed" << std::endl;
    return false;
  }

  if (util::clamp(-5, 0, 10) != 0) {
    std::cerr << "clamp test 2 failed" << std::endl;
    return false;
  }

  if (util::clamp(15, 0, 10) != 10) {
    std::cerr << "clamp test 3 failed" << std::endl;
    return false;
  }

  return true;
}

bool test_distance() {
  Vec2f a{0.0f, 0.0f};
  Vec2f b{3.0f, 4.0f};

  double dist = util::dist(a, b);
  if (std::abs(dist - 5.0) > 0.001) {
    std::cerr << "distance test failed: expected 5.0, got " << dist
              << std::endl;
    return false;
  }

  return true;
}

int main(int argc, char *argv[]) {
  bool all_passed = true;

  if (!test_map_range()) {
    all_passed = false;
  }

  if (!test_clamp()) {
    all_passed = false;
  }

  if (!test_distance()) {
    all_passed = false;
  }

  if (all_passed) {
    std::cout << "All MathUtil tests passed!" << std::endl;
    return 0;
  } else {
    std::cerr << "Some tests failed!" << std::endl;
    return 1;
  }
}