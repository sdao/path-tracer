#include "sphere.h"

sphere::sphere(vec o, float r, materialptr m)
  : origin(o), radius(r), geom(m) {}

sphere::sphere(vec o, float r)
  : origin(o), radius(r), geom() {}

sphere::sphere() : origin(0.0), radius(1.0f) {}

sphere::~sphere() {}

intersection sphere::intersect(const ray& r) const {
  vec diff = r.origin - origin;
  vec l = r.unit().direction;

  // See Wikipedia:
  // <http://en.wikipedia.org/wiki/Line%E2%80%93sphere_intersection>
  float a = glm::dot(l, l);
  float b = glm::dot(l, diff);
  float c = glm::dot(diff, diff) - (radius * radius);

  double discriminant = (b * b) - (a * c);

  if (discriminant > 0.0f) {
    discriminant = sqrt(discriminant);
    // Quadratic has at most 2 results.
    float resPos = (-b + discriminant);
    float resNeg = (-b - discriminant);
    float epsilon = std::numeric_limits<float>::epsilon();

    // Neg before pos because we want to return closest isect first.
    if (resNeg > epsilon) {
      vec pt = r.unit().at(resNeg);
      vec normal = glm::normalize(pt - origin);
      return intersection(pt, normal, resNeg);
    } else if (resPos > epsilon) {
      vec pt = r.unit().at(resPos);
      vec normal = glm::normalize(pt - origin);
      return intersection(pt, normal, resPos);
    }
  }

  // Either no isect was found or it was behind us.
  return intersection();
}
