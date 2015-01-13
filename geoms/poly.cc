#include "poly.h"

geoms::Poly::Poly(
  ID a,
  ID b,
  ID c,
  std::vector<geoms::Poly::Point>* lookup,
  const Material* m,
  const AreaLight* l
) : Geom(m, l), pt0(a), pt1(b), pt2(c), pointLookup(lookup) {}

geoms::Poly::Poly(const geoms::Poly& other)
  : Geom(other.mat, other.light), pt0(other.pt0), pt1(other.pt1), pt2(other.pt2),
    pointLookup(other.pointLookup) {}

bool geoms::Poly::intersect(const Ray& r, Intersection* isectOut) const {
  // Uses the Moller-Trumbore intersection algorithm.
  // See <http://en.wikipedia.org/wiki/
  // M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm> for more info.

  const Point& data0 = getPt0();
  const Point& data1 = getPt1();
  const Point& data2 = getPt2();

  const Vec edge1 = data1.position - data0.position;
  const Vec edge2 = data2.position - data0.position;

  const Vec p = r.direction.cross(edge2);
  const float det = edge1.dot(p);

  if (math::isNearlyZero(det)) {
    return false; // No hit on plane.
  }

  const float invDet = 1.0f / det;
  const Vec t = r.origin - data0.position;

  const float u = t.dot(p) * invDet;
  if (u < 0.0f || u > 1.0f) {
    return false; // In plane but not triangle.
  }

  const Vec q = t.cross(edge1);
  const float v = r.direction.dot(q) * invDet;
  if (v < 0.0f || (u + v) > 1.0f) {
    return false; // In plane but not triangle.
  }

  const float dist = edge2.dot(q) * invDet;
  if (!math::isPositive(dist)) {
    return false; // In triangle but behind us.
  }

  const float w = 1.0f - u - v;

  Intersection isect;
  isect.position = r.at(dist);
  isect.distance = dist;
  isect.normal =
    w * data0.normal + u * data1.normal + v * data2.normal;

  *isectOut = isect;
  return true;
}

bool geoms::Poly::intersectShadow(const Ray& r, float maxDist) const {
  // Uses the Moller-Trumbore intersection algorithm.
  // See <http://en.wikipedia.org/wiki/
  // M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm> for more info.

  const Point& data0 = getPt0();
  const Point& data1 = getPt1();
  const Point& data2 = getPt2();

  const Vec edge1 = data1.position - data0.position;
  const Vec edge2 = data2.position - data0.position;

  const Vec p = r.direction.cross(edge2);
  const float det = edge1.dot(p);

  if (math::isNearlyZero(det)) {
    return false; // No hit on plane.
  }

  const float invDet = 1.0f / det;
  const Vec t = r.origin - data0.position;

  const float u = t.dot(p) * invDet;
  if (u < 0.0f || u > 1.0f) {
    return false; // In plane but not triangle.
  }

  const Vec q = t.cross(edge1);
  const float v = r.direction.dot(q) * invDet;
  if (v < 0.0f || (u + v) > 1.0f) {
    return false; // In plane but not triangle.
  }

  const float dist = edge2.dot(q) * invDet;
  if (!math::isPositive(dist) || !math::isPositive(maxDist - dist)) {
    return false; // In triangle but not in the right range.
  }

  return true;
}

BBox geoms::Poly::bounds() const {
  BBox b(getPt0().position, getPt1().position);
  b.expand(getPt2().position);

  return b;
}

Vec geoms::Poly::samplePoint(Randomness& rng) const {
  // See MathWorld <http://mathworld.wolfram.com/TrianglePointPicking.html>.
  // Verified working in Mathematica.
  float a = rng.nextUnitFloat();
  float b = rng.nextUnitFloat();
  if (a + b > 1.0f) {
    a = 1.0f - a;
    b = 1.0f - b;
  }
  float c = 1.0f - a - b;

  return a * getPt0().position + b * getPt1().position + c * getPt2().position;
}

float geoms::Poly::area() const {
  // See MathWorld <http://mathworld.wolfram.com/TriangleArea.html>.
  Vec x0 = getPt0().position;
  Vec x1 = getPt1().position;
  Vec x2 = getPt2().position;

  return 0.5f * (x1 - x0).cross(x0 - x2).norm();
}
