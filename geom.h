#pragma once
#include "math.h"
#include "material.h"

class geom {
protected:
  geom() : mat() {}
  geom(material m) : mat(m) {}

public:
  material mat;
  
  virtual ~geom() {}
  virtual float intersect(const ray& r) const = 0;
};
