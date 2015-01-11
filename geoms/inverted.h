#pragma once
#include "../geom.h"

namespace geoms {

  /**
   * A wrapper that simply inverts the normals of another geometry object.
   */
  class Inverted : public Geom {
  public:
    /** The original (uninverted) geometry. */
    const Geom* original;

    /** Constructs an inverted version of the given geometry object. */
    Inverted(const Geom* g);

    /** Constructs an inverted geometry object from the given node. */
    Inverted(const Node& n);

    virtual bool intersect(const Ray& r, Intersection* isectOut) const override;
    virtual bool intersectShadow(const Ray& r, float maxDist) const override;
    virtual BBox bounds() const override;
    virtual BSphere boundSphere() const override;
    virtual Vec samplePoint(Randomness& rng) const override;
    virtual float area() const override;
  };

}
