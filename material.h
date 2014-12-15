#pragma once
#include <memory>
#include <limits>
#include "math.h"

#define IOR_VACUUM 1.0f

class material {
protected:
  material(vec d) : debug_color(d) {}
  material() : debug_color(vec(1)) {}

public:
  vec debug_color;
  virtual ~material() {}
  virtual lightray propagate(
    const lightray& incoming,
    intersection& isect,
    randomness& rng
  ) const = 0;
};

typedef std::shared_ptr<material> materialptr;
