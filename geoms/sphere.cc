#include "sphere.h"

geoms::Sphere::Sphere(Vec o, float r, Material* m, AreaLight* l)
  : Geom(m, l), origin(o), radius(r) {}

geoms::Sphere::Sphere(const geoms::Sphere& other)
  : Geom(other.mat, other.light), origin(other.origin), radius(other.radius) {}

bool geoms::Sphere::intersect(const Ray& r, Intersection* isectOut) const {
  Vec diff = r.origin - origin;
  Vec l = r.direction;

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
      Vec pt = r.at(resNeg);
      Vec normal = (pt - origin).normalized();

      *isectOut = Intersection(pt, normal, resNeg);
      return true;
    } else if (math::isPositive(resPos)) {
      Vec pt = r.at(resPos);
      Vec normal = (pt - origin).normalized();

      *isectOut = Intersection(pt, normal, resPos);
      return true;
    }
  }

  // Either no isect was found or it was behind us.
  return false;
}

BBox geoms::Sphere::bounds() const {
  Vec boundsDiag(radius, radius, radius);
  return BBox(origin - boundsDiag, origin + boundsDiag);
}

Vec geoms::Sphere::samplePoint(Randomness& rng) const {
  // See MathWorld <http://mathworld.wolfram.com/SpherePointPicking.html>.
  float x = rng.nextNormalFloat();
  float y = rng.nextNormalFloat();
  float z = rng.nextNormalFloat();
  float scale = radius / sqrtf(x * x + y * y + z * z);

  return Vec(
    origin.x() + scale * x,
    origin.y() + scale * y,
    origin.z() + scale * z
  );
}

float geoms::Sphere::area() const {
  return 4.0f * math::PI * radius * radius;
}
