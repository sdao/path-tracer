#pragma once
#include "../material.h"

namespace materials {

  /**
   * A glossy reflective material using Phong reflectance.
   * Note that Phong reflectance is not physically plausible.
   */
  class Phong : public Material {
    /** Cached scaling term in the Phong BRDF. */
    const Vec scaleBRDF;

    /** Cached scaling term in the PDF. */
    const float scaleProb;

    /** Cached inverse exponent term. */
    const float invExponent;

    inline Vec evalBSDFInternal(
      const Vec& perfectReflect,
      const Vec& outgoing
    ) const;

    inline float evalPDFInternal(
      const Vec& perfectReflect,
      const Vec& outgoing
    ) const;

  protected:
    virtual Vec evalBSDFLocal(
      const Vec& incoming,
      const Vec& outgoing
    ) const override;

    virtual float evalPDFLocal(
      const Vec& incoming,
      const Vec& outgoing
    ) const override;

  public:
    /** The Phong exponent of the material. */
    const float exponent;

    /** The color of the material. */
    const Vec color;

    /**
     * Constructs a Phong material.
     *
     * @param e the Phong exponent (e >= 1); 1 = almost perfectly diffuse,
     *          ~1000 = almost perfectly specular, lower values are more glossy
     * @param c the color of the material
     */
    Phong(float e, Vec c = Vec(1, 1, 1));

    virtual void sampleLocal(
      Randomness& rng,
      const Vec& incoming,
      Vec* outgoingOut,
      Vec* bsdfOut,
      float* pdfOut
    ) const override;

    virtual bool shouldDirectIlluminate() const override;
  };

}
