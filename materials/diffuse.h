#pragma once
#include "../material.h"

namespace materials {

  /**
   * A perfectly-ideal diffuse reflector.
   */
  class diffuse : public material {
  public:
    const vec color; /**< The diffuse color. */

    /** Constructs a diffuse material with the given surface color. */
    diffuse(vec c);

    virtual lightray propagate(
      const lightray& incoming,
      intersection& isect,
      randomness& rng
    ) const;
  };

}
