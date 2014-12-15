#pragma once
#include "../material.h"

namespace materials {

  class fresnel : public material {
  public:
    const float idxRefract;

    fresnel(float ior = 1.5f);

    virtual lightray propagate(
      const lightray& incoming,
      intersection& isect,
      randomness& rng
    ) const;

    static materialptr make(float ior = 1.5f);
  };

}
