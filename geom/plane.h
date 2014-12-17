#pragma once
#include "geom.h"

class plane : public geom {
public:
  vec origin;
  vec normal;

  plane(vec o, vec n, material* m);
  plane(vec o, vec n);
  plane();
  ~plane();

  virtual intersection intersect(const ray& r) const;
};
