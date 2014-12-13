#pragma once
#include <cmath>
#include <limits>
#include <glm/glm.hpp>

typedef glm::vec3 vec;

struct ray {
  vec origin;
  vec direction;

  ray(vec o, vec d) : origin(o), direction(d) {}
  ray() : origin(0.0), direction(0.0) {}

  vec unitDirection() const { return glm::normalize(direction); }
};

namespace math {

  inline float clamp(float x) {
    return x < 0 ? 0 : (x > 1 ? 1 : x);
  }

}
