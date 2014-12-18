#pragma once
#include "../material.h"

namespace materials {

  class fresnel : public material {
    static constexpr float IOR_VACUUM = 1.0f;

    float r0;
    const float etaEntering; // nAir / nThisMaterial
    const float etaExiting; // nThisMaterial / nAir;

  public:
    static constexpr float IOR_GLASS = 1.5f;
    static constexpr float IOR_DIAMOND = 2.4f;
  
    fresnel(float ior = IOR_GLASS);

    virtual lightray propagate(
      const lightray& incoming,
      intersection& isect,
      randomness& rng
    ) const;
  };

}
