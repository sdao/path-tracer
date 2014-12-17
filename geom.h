#pragma once
#include "math.h"
#include "material.h"

class geom {
protected:
  geom(material* m);

public:
  material* mat;

  virtual ~geom();
  virtual intersection intersect(const ray& r) const = 0;
  virtual bbox bounds() const = 0;
};
