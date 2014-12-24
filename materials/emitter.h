#pragma once
#include "../material.h"

namespace materials {

  /**
   * An emissive light source.
   */
  class Emitter : public Material {
  public:
    const Vec color; /**< The color of the emitted light. */

    /** Constructs an emitter material that emits the given color light. */
    Emitter(Vec c);

    virtual LightRay propagate(
      const LightRay& incoming,
      const Intersection& isect,
      Randomness& rng
    ) const override;
  };

}
