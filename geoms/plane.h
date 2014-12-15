#pragma once
#include "../geom.h"

namespace geoms {

  class plane : public geom {
    vec tangent;
    vec cotangent;

    void computeTangents();

  public:
    const vec origin;
    const vec normal;

    plane(vec o, vec n, materialptr m);
    plane(vec o, vec n);
    plane();

    virtual intersection intersect(const ray& r) const;

    static geomptr make(vec o, vec n, materialptr m);
    static geomptr make(vec o, vec n);
    static geomptr make();
  };

}
