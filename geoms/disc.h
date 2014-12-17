#pragma once
#include "../geom.h"

namespace geoms {

  class disc : public geom {
    vec tangent;
    vec cotangent;
    const float radiusSquared;

  public:
    const vec origin;
    const vec normal;
    const float radius;

    disc(
      material* m,
      vec o = vec(0),
      vec n = vec(0, 1, 0),
      float r = 10.0f
    );

    virtual intersection intersect(const ray& r) const;
    virtual bbox bounds() const;

    static geom* make(
      material* m,
      vec o = vec(0),
      vec n = vec(0, 1, 0),
      float r = 10.0f
    );
  };

}
