#pragma once
#include "geom.h"

class sphere : public geom {
public:
  vec origin;
  float radius;

  sphere(vec o, float r, material m);
  sphere(vec o, float r);
  sphere();
  ~sphere();

  virtual float intersect(const ray& r) const;
};
