#pragma once
#include "../material.h"

namespace materials {

  /**
   * A perfectly-specular reflector.
   */
  class specular : public material {
  public:
    /**
     * Constructs a specular material.
     */
    specular();

    virtual lightray propagate(
      const lightray& incoming,
      intersection& isect,
      randomness& rng
    ) const;
  };

}
