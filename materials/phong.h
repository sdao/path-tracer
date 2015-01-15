#pragma once
#include "../material.h"

namespace materials {

  /**
   * A glossy reflective material using Phong reflectance.
   * Note that Phong reflectance is not physically plausible.
   */
  class Phong : public Material {
    const Vec scaleBRDF; /**< Cached scaling term in the Phong BRDF. */
    const float scaleProb; /**< Cached scaling term in the PDF. */
    const float invExponent; /**< Cached inverse exponent term. */

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
    /** The color of the material. */
    const Vec color;

    /** The Phong exponent of the material. */
    const float exponent;

    /**
     * Constructs a Phong material.
     *
     * @param e the Phong exponent (e >= 1); 1 = almost perfectly diffuse,
     *          ~1000 = almost perfectly specular, lower values are more glossy
     * @param c the color of the material
     */
    Phong(float e, const Vec& c = Vec(1, 1, 1));
    
    /**
     * Constructs a Phong material from the given node.
     */
    Phong(const Node& n);

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
