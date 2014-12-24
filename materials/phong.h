#pragma once
#include "../material.h"

namespace materials {

  /**
   * A glossy reflective material using Phong shading.
   * Note that Phong shading is not physically plausible.
   */
  class phong : public material {
  protected:
    virtual vec sampleBSDF(
      randomness& rng,
      const vec& incoming,
      vec* outgoingOut,
      float* probabilityOut
    ) const override;

    virtual vec evalBSDF(
      const vec& incoming,
      const vec& outgoing
    ) const override;

    /** Cached scaling term in the Phong BRDF. */
    const vec scaleBRDF;

    /** Cached scaling term in the PDF. */
    const float scaleProb;

    /** Cached inverse exponent term. */
    const float invExponent;

  public:
    /** The Phong exponent of the material. */
    const float exponent;

    /** The color of the material. */
    const vec color;

    /**
     * Constructs a Phong material.
     *
     * @param e the Phong exponent (e >= 1); 1 = almost perfectly diffuse,
     *          ~1000 = almost perfectly specular, lower values are more glossy
     * @param c the color of the material
     */
    phong(float e, vec c = vec(1, 1, 1));
  };

}
