#pragma once
#include "../material.h"

namespace materials {

  /**
   * A glossy reflective material whose reflections are more spread-out
   * than an ideal specular material's but less spread-out than an ideal
   * diffuse material's.
   */
  class glossy : public material {
  public:
    /**
     * The glossiness of the material. 0 = perfectly specular, 1 = perfectly
     * diffuse.
     */
    const float glossiness;

    /**
     * Constructs a glossy material.
     *
     * @param g the glossiness of the material. Must be between 0 and 1;
     *          0 = perfectly specular, 1 = perfectly diffuse
     */
    glossy(float g);

    virtual lightray propagate(
      const lightray& incoming,
      const intersection& isect,
      randomness& rng
    ) const;
  };

}
