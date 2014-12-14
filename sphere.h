#pragma once
#include "geom.h"

class sphere : public geom {
public:
  vec origin;
  float radius;

  sphere(vec o, float r, materialptr m);
  sphere(vec o, float r);
  sphere();
  ~sphere();

  virtual intersection intersect(const ray& r) const;
};
