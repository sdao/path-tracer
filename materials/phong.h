#pragma once
#include "../material.h"

namespace materials {

  /**
   * A glossy reflective material using Phong shading.
   * Note that Phong shading is not physically plausible.
   */
  class phong : public material {
  public:
    /**
     * The inverse of the Phong exponent of the material.
     */
    const float invExponent;

    /**
     * Constructs a glossy material.
     *
     * @param e the Phong exponent (e >= 1); 1 = almost perfectly diffuse,
     *          ~1000 = almost perfectly specular, lower values are more glossy
     */
    phong(float e);

    virtual lightray propagate(
      const lightray& incoming,
      const intersection& isect,
      randomness& rng
    ) const override;
  };

}
