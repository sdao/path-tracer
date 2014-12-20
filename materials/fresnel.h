#pragma once
#include "../material.h"

namespace materials {

  /**
   * A dielectric (reflective/refractive) material following the Fresnel
   * equations.
   */
  class fresnel : public material {
    /** The refractive index for a vacuum (approx. air), nVac. */
    static constexpr float IOR_VACUUM = 1.0f;

    float r0; /** The cached R(0) value for Schlick's approximation. */
    const float etaEntering; /** The refraction ratio nVac / nMaterial. */
    const float etaExiting; /** Inverse of etaEntering (nMaterial / nVac). */

  public:
    static constexpr float IOR_GLASS = 1.5f; /**< The IOR for glass. */
    static constexpr float IOR_DIAMOND = 2.4f; /**< The IOR for diamond. */

    /** Creates a Fresnel material. */
    fresnel(float ior = IOR_GLASS);

    virtual lightray propagate(
      const lightray& incoming,
      const intersection& isect,
      randomness& rng
    ) const;
  };

}
