#pragma once
#include <memory>
#include "math.h"

class material {
protected:
  material(vec d) : debug_color(d) {}

public:
  vec debug_color;
  virtual ~material() {}
  virtual ray propagate(const ray& incoming, const intersection& isect) = 0;
};

typedef std::shared_ptr<material> materialptr;

class testmaterial : public material {
public:
  testmaterial(vec d) : material(d) {}
  virtual ray propagate(const ray& incoming, const intersection& isect) {
    return ray();
  }
};
