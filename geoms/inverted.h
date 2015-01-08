#pragma once
#include "../geom.h"

namespace geoms {

  class Inverted : public Geom {
  public:
    const Geom* original;

    Inverted(const Geom* g);

    virtual bool intersect(const Ray& r, Intersection* isectOut) const override;
    virtual bool intersectShadow(const Ray& r, float maxDist) const override;
    virtual BBox bounds() const override;
    virtual BSphere boundSphere() const override;
    virtual Vec samplePoint(Randomness& rng) const override;
    virtual float area() const override;
  };

}
