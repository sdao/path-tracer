#include "inverted.h"

geoms::Inverted::Inverted(const Geom* g)
  : Geom(g->mat, g->light), original(g) {}

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

Vec geoms::Inverted::samplePoint(Randomness& rng) const {
  return original->samplePoint(rng);
}

float geoms::Inverted::area() const {
  return original->area();
}
