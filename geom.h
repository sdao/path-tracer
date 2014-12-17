#pragma once
#include <memory>
#include "math.h"
#include "material.h"

class geom {
protected:
  geom(materialptr m);

public:
  materialptr mat;

  virtual ~geom();
  virtual intersection intersect(const ray& r) const = 0;
  virtual bbox bounds() const = 0;
};

typedef std::shared_ptr<geom> geomptr;
