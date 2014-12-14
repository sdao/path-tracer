#include "plane.h"

plane::plane(vec o, vec n, materialptr m)
  : origin(o), normal(glm::normalize(n)), geom(m) {}

plane::plane(vec o, vec n)
  : origin(o), normal(glm::normalize(n)), geom() {}

plane::plane() : origin(0.0), normal(0.0, 1.0, 0.0) {}

plane::~plane() {}

intersection plane::intersect(const ray& r) const {
  // See Wikipedia:
  // <http://en.wikipedia.org/wiki/Line%E2%80%93plane_intersection>

  float denom = glm::dot(r.unit().direction, normal);
  if (denom != 0.0f) {
    float d = glm::dot(origin - r.origin, normal) / denom;
    float epsilon = std::numeric_limits<float>::epsilon();

    if (d > epsilon) {
      return intersection(r.unit().at(d), normal, d);
    }
  }

  // Either no isect was found or it was behind us.
  return intersection();
}
