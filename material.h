#pragma once
#include <memory>
#include <limits>
#include "math.h"

class material {
protected:
  material(vec d);
  material();

public:
  virtual ~material();
  virtual lightray propagate(
    const lightray& incoming,
    intersection& isect,
    randomness& rng
  ) const = 0;
};

typedef std::shared_ptr<material> materialptr;
