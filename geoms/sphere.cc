#include "sphere.h"

geoms::sphere::sphere(material* m, vec o, float r)
  : geom(m), origin(o), radius(r) {}

geoms::sphere::sphere(const geoms::sphere& other)
  : geom(other.mat), origin(other.origin), radius(other.radius) {}

bool geoms::sphere::intersect(const ray& r, intersection* isectOut) const {
  vec diff = r.origin - origin;
  vec l = r.direction;

  // See Wikipedia:
  // <http://en.wikipedia.org/wiki/Line%E2%80%93sphere_intersection>
  float a = l.dot(l);
  float b = l.dot(diff);
  float c = diff.dot(diff) - (radius * radius);

  float discriminant = (b * b) - (a * c);

  if (discriminant > 0.0f) {
    discriminant = sqrtf(discriminant);
    // Quadratic has at most 2 results.
    float resPos = (-b + discriminant);
    float resNeg = (-b - discriminant);

    // Neg before pos because we want to return closest isect first.
    if (math::isPositive(resNeg)) {
      vec pt = r.at(resNeg);
      vec normal = (pt - origin).normalized();

      *isectOut = intersection(pt, normal, resNeg);
      return true;
    } else if (math::isPositive(resPos)) {
      vec pt = r.at(resPos);
      vec normal = (pt - origin).normalized();

      *isectOut = intersection(pt, normal, resPos);
      return true;
    }
  }

  // Either no isect was found or it was behind us.
  return false;
}

bbox geoms::sphere::bounds() const {
  vec boundsDiag(radius, radius, radius);
  return bbox(origin - boundsDiag, origin + boundsDiag);
}
