#include "sphere.h"

geoms::Sphere::Sphere(
  const Vec& o, float r, bool i, const Material* m, const AreaLight* l
) : Geom(m, l), origin(o), radius(r), inverted(i) {}

geoms::Sphere::Sphere(const geoms::Sphere& other)
  : Geom(other.mat, other.light),
    origin(other.origin), radius(other.radius), inverted(other.inverted) {}

geoms::Sphere::Sphere(const Node& n)
  : Sphere(n.getVec("origin"), n.getFloat("radius"), n.getBool("inverted"),
           n.getMaterial("mat"), n.getLight("light")) {}

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
      Vec normal = (inverted ? origin - pt : pt - origin).normalized();

      *isectOut = Intersection(pt, normal, resNeg);
      return true;
    } else if (math::isPositive(resPos)) {
      Vec pt = r.at(resPos);
      Vec normal = (inverted ? origin - pt : pt - origin).normalized();

      *isectOut = Intersection(pt, normal, resPos);
      return true;
    }
  }

  // Either no isect was found or it was behind us.
  return false;
}

bool geoms::Sphere::intersectShadow(const Ray& r, float maxDist) const {
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
    if (math::isPositive(resNeg) && math::isPositive(maxDist - resNeg)) {
      return true;
    } else if (math::isPositive(resPos) && math::isPositive(maxDist - resPos)) {
      return true;
    }
  }

  // Either no isect was found or it was not in the right range.
  return false;
}

BBox geoms::Sphere::boundBox() const {
  Vec boundsDiag(radius, radius, radius);
  return BBox(origin - boundsDiag, origin + boundsDiag);
}

BSphere geoms::Sphere::boundSphere() const {
  return BSphere(origin, radius);
}
