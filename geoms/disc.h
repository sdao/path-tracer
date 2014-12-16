#pragma once
#include "../geom.h"

namespace geoms {

  class disc : public geom {
    vec tangent;
    vec cotangent;

    void finishConstructing();

  public:
    const vec origin;
    const vec normal;
    const float radiusSquared;

    disc(
      materialptr m,
      vec o = vec(0),
      vec n = vec(0, 1, 0),
      float r = 10.0f
    );

    virtual intersection intersect(const ray& r) const;

    static geomptr make(
      materialptr m,
      vec o = vec(0),
      vec n = vec(0, 1, 0),
      float r = 10.0f
    );
  };

}
