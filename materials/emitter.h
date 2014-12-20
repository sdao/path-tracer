#pragma once
#include "../material.h"

namespace materials {

  /**
   * An emissive light source.
   */
  class emitter : public material {
  public:
    const vec color; /**< The color of the emitted light. */

    /** Constructs an emitter material that emits the given color light. */
    emitter(vec c);

    virtual lightray propagate(
      const lightray& incoming,
      const intersection& isect,
      randomness& rng
    ) const;
  };

}
