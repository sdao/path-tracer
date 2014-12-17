#include "sphere.h"

geoms::sphere::sphere(material* m, vec o, float r)
  : geom(m), origin(o), radius(r) {}

intersection geoms::sphere::intersect(const ray& r) const {
  vec diff = r.origin - origin;
  vec l = r.direction;

  // See Wikipedia:
  // <http://en.wikipedia.org/wiki/Line%E2%80%93sphere_intersection>
  float a = glm::dot(l, l);
  float b = glm::dot(l, diff);
  float c = glm::dot(diff, diff) - (radius * radius);

  float discriminant = (b * b) - (a * c);

  if (discriminant > 0.0f) {
    discriminant = sqrtf(discriminant);
    // Quadratic has at most 2 results.
    float resPos = (-b + discriminant);
    float resNeg = (-b - discriminant);

    // Neg before pos because we want to return closest isect first.
    if (math::isPositive(resNeg)) {
      vec pt = r.at(resNeg);
      vec normal = glm::normalize(pt - origin);
      return intersection(pt, normal, resNeg);
    } else if (math::isPositive(resPos)) {
      vec pt = r.at(resPos);
      vec normal = glm::normalize(pt - origin);
      return intersection(pt, normal, resPos);
    }
  }

  // Either no isect was found or it was behind us.
  return intersection();
}

bbox geoms::sphere::bounds() const {
  vec boundsDiag(radius, radius, radius);
  return bbox(origin - boundsDiag, origin + boundsDiag);
}
