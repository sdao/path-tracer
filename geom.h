#pragma once
#include <memory>
#include "math.h"
#include "material.h"

class geom {
protected:
  geom() : mat() {}
  geom(materialptr m) : mat(m) {}

public:
  materialptr mat;

  virtual ~geom() {}
  virtual intersection intersect(const ray& r) const = 0;
};

typedef std::shared_ptr<geom> geomptr;
