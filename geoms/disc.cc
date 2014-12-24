#include "disc.h"

geoms::Disc::Disc(Material* m, Vec o, Vec n, float r)
  : Geom(m), radiusSquared(r * r), radius(r), origin(o),
    normal(n.normalized()) {}

geoms::Disc::Disc(const geoms::Disc& other)
  : Geom(other.mat), radiusSquared(other.radiusSquared), radius(other.radius),
    origin(other.origin), normal(other.normal) {}

bool geoms::Disc::intersect(const Ray& r, Intersection* isectOut) const {
  // See Wikipedia:
  // <http://en.wikipedia.org/wiki/Line%E2%80%93disc_intersection>

  float denom = r.direction.dot(normal);
  if (denom != 0.0f) {
    float d = (origin - r.origin).dot(normal) / denom;

    if (math::isPositive(d)) {
      // In the plane, but are we in the disc?
      Vec isectPoint = r.at(d);
      if ((isectPoint - origin).squaredNorm() < radiusSquared) {
        // In the disc.
        *isectOut = Intersection(isectPoint, normal, d);

        return true;
      }
    }
  }

  // Either no isect was found or it was behind us.
  return false;
}

BBox geoms::Disc::bounds() const {
  Vec tangent;
  Vec binormal;
  math::coordSystem(normal, &tangent, &binormal);

  Vec tr = tangent * radius;
  Vec br = binormal * radius;

  BBox b(origin + tr + br, origin - tr - br);
  b.expand(origin + tr - br);
  b.expand(origin - tr + br);

  return b;
}
