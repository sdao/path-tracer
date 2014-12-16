#pragma once
#include "../geom.h"

namespace geoms {

  class sphere : public geom {
    void finishConstructing();

  public:
    const vec origin;
    const float radius;

    sphere(materialptr m, vec o = vec(0), float r = 1.0f);

    virtual intersection intersect(const ray& r) const;

    static geomptr make(materialptr m, vec o = vec(0), float r = 1.0f);
  };

}
