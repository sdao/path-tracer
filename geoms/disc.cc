#include "disc.h"

geoms::disc::disc(material* m, vec o, vec n, float r)
  : geom(m), radiusSquared(r * r), origin(o), normal(glm::normalize(n)),
    radius(r)
{
  math::coordSystem(normal, &tangent, &binormal);
}

geoms::disc::disc(const geoms::disc& other)
  : geom(other.mat), radiusSquared(other.radiusSquared),
    origin(other.origin), normal(other.normal), radius(other.radius) {}

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
        return intersection(isectPoint, normal, tangent, binormal, d);
      }
    }
  }

  // Either no isect was found or it was behind us.
  return intersection();
}

bbox geoms::disc::bounds() const {
  vec tr = tangent * radius;
  vec br = binormal * radius;

  bbox b(origin + tr + br, origin - tr - br);
  b.expand(origin + tr - br);
  b.expand(origin - tr + br);

  return b;
}
