#pragma once
#include "../geom.h"

namespace geoms {

  class sphere : public geom {
  public:
    const vec origin;
    const float radius;

    sphere(vec o, float r, materialptr m);
    sphere(vec o, float r);
    sphere();

    virtual intersection intersect(const ray& r) const;

    static geomptr make(vec o, float r, materialptr m);
    static geomptr make(vec o, float r);
    static geomptr make();
  };

}
