#include "inverted.h"

geoms::Inverted::Inverted(const Geom* g)
  : Geom(g->mat, g->light), original(g) {}

geoms::Inverted::Inverted(const Node& n)
  : Inverted(n.getGeometry("original")) {}

bool geoms::Inverted::intersect(const Ray& r, Intersection* isectOut) const {
  bool status = original->intersect(r, isectOut);
  isectOut->normal = isectOut->normal * -1.0f;
  return status;
}

bool geoms::Inverted::intersectShadow(const Ray& r, float maxDist) const {
  return original->intersectShadow(r, maxDist);
}

BBox geoms::Inverted::bounds() const {
  return original->bounds();
}

BSphere geoms::Inverted::boundSphere() const {
  return original->boundSphere();
}
