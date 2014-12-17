#pragma once
#include "../material.h"

namespace materials {

  class fresnel : public material {
    static constexpr float IOR_VACUUM = 1.0f;

    float r0;
    const float etaEntering; // nAir / nThisMaterial
    const float etaExiting; // nThisMaterial / nAir;

  public:
    fresnel(float ior = 1.5f);

    virtual lightray propagate(
      const lightray& incoming,
      intersection& isect,
      randomness& rng
    ) const;
  };

}
