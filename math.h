#pragma once
#include <cmath>
#include <limits>
#include <glm/glm.hpp>

typedef glm::vec3 vec;

struct ray {
  vec origin;
  vec direction;
  float wavelength;
  float probability;

  ray(vec o, vec d) : origin(o), direction(d) {}
  ray() : origin(0.0), direction(0.0) {}

  ray unit() const { return ray(origin, glm::normalize(direction)); }
  vec at(float d) const { return origin + direction * d; }
};

struct intersection {
  vec position;
  vec normal;
  vec tangent;
  float distance;

  intersection() : position(0), normal(0), tangent(0), distance(0.0f) {}
  intersection(vec p, float d)
    : position(p), normal(0), tangent(0), distance(d) {}
  bool hit() const { return distance > 0.0f; }
};

namespace math {

  inline float clamp(float x) {
    return x < 0 ? 0 : (x > 1 ? 1 : x);
  }

}
