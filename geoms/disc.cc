#include "disc.h"
#include <memory>

geoms::disc::disc(materialptr m, vec o, vec n, float r)
  : geom(m), origin(o), normal(glm::normalize(n)), radiusSquared(r * r)
{
  finishConstructing();
}

void geoms::disc::finishConstructing() {
  math::coordSystem(normal, &tangent, &cotangent);
  bounds = bbox(origin + tangent + cotangent, origin - tangent - cotangent);
  bounds.expand(origin + tangent - cotangent);
  bounds.expand(origin - tangent + cotangent);
}

intersection geoms::disc::intersect(const ray& r) const {
  // See Wikipedia:
  // <http://en.wikipedia.org/wiki/Line%E2%80%93disc_intersection>

  float denom = glm::dot(r.direction, normal);
  if (denom != 0.0f) {
    float d = glm::dot(origin - r.origin, normal) / denom;

    if (math::isPositive(d)) {
      // In the plane, but are we in the disc?
      vec isectPoint = r.at(d);
      if (glm::length2(isectPoint - origin) < radiusSquared) {
        // In the disc.
        return intersection(isectPoint, normal, tangent, cotangent, d);
      }
    }
  }

  // Either no isect was found or it was behind us.
  return intersection();
}

geomptr geoms::disc::make(materialptr m, vec o, vec n, float r) {
  return std::make_shared<disc>(m, o, n, r);
}
