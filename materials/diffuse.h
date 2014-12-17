#pragma once
#include "../material.h"

namespace materials {

  class diffuse : public material {
  public:
    const vec color;

    diffuse(vec c);

    virtual lightray propagate(
      const lightray& incoming,
      intersection& isect,
      randomness& rng
    ) const;
  };

}
