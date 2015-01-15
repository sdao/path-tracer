#include "disc.h"

geoms::Disc::Disc(
  const Vec& o,
  const Vec& n,
  float rOuter,
  float rInner,
  const Material* m,
  const AreaLight* l
) : Geom(m, l),
    radiusOuterSquared(rOuter * rOuter), radiusInnerSquared(rInner * rInner),
    radiusOuter(rOuter), radiusInner(rInner),
    origin(o), normal(n.normalized()) {}

geoms::Disc::Disc(const geoms::Disc& other)
  : Geom(other.mat, other.light),
    radiusOuterSquared(other.radiusOuterSquared),
    radiusInnerSquared(other.radiusInnerSquared),
    radiusOuter(other.radiusOuter),
    radiusInner(other.radiusInner),
    origin(other.origin), normal(other.normal) {}

geoms::Disc::Disc(const Node& n)
  : Disc(n.getVec("origin"), n.getVec("normal"),
         n.getFloat("radiusOuter"), n.getFloat("radiusInner"),
         n.getMaterial("mat"), n.getLight("light")) {}

bool geoms::Disc::intersect(const Ray& r, Intersection* isectOut) const {
  // See Wikipedia:
  // <http://en.wikipedia.org/wiki/Line%E2%80%93disc_intersection>

  float denom = r.direction.dot(normal);
  if (denom != 0.0f) {
    float d = (origin - r.origin).dot(normal) / denom;

    if (math::isPositive(d)) {
      // In the plane, but are we in the disc?
      Vec isectPoint = r.at(d);
      float isectToOriginDist = (isectPoint - origin).squaredNorm();
      if (isectToOriginDist <= radiusOuterSquared
          && isectToOriginDist >= radiusInnerSquared) {
        // In the disc.
        *isectOut = Intersection(isectPoint, normal, d);

        return true;
      }
    }
  }

  // Either no isect was found or it was behind us.
  return false;
}

bool geoms::Disc::intersectShadow(const Ray& r, float maxDist) const {
  // See Wikipedia:
  // <http://en.wikipedia.org/wiki/Line%E2%80%93disc_intersection>

  float denom = r.direction.dot(normal);
  if (denom != 0.0f) {
    float d = (origin - r.origin).dot(normal) / denom;

    if (math::isPositive(d) && math::isPositive(maxDist - d)) {
      // In the plane (and in range), but are we in the disc?
      Vec isectPoint = r.at(d);
      float isectToOriginDist = (isectPoint - origin).squaredNorm();
      if (isectToOriginDist <= radiusOuterSquared
          && isectToOriginDist >= radiusInnerSquared) {
        // In the disc.
        return true;
      }
    }
  }

  // Either no isect was found or it was behind us.
  return false;
}

BBox geoms::Disc::boundBox() const {
  Vec tangent;
  Vec binormal;
  math::coordSystem(normal, &tangent, &binormal);

  Vec tr = tangent * radiusOuter;
  Vec br = binormal * radiusOuter;

  BBox b(origin + tr + br, origin - tr - br);
  b.expand(origin + tr - br);
  b.expand(origin - tr + br);

  return b;
}

BSphere geoms::Disc::boundSphere() const {
  return BSphere(origin, radiusOuter);
}
