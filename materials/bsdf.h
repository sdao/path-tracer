#pragma once
#include "../material.h"

namespace materials {

  /**
   * The abstract base class for all reflective/transmissive materials that
   * follow a bidirectional scattering distribution function (BSDF), namely a
   * BRDF or a BTDF.
   */
  class BSDF : public Material {
  protected:
    /**
     * Samples the BSDF at a random output direction. The sampling need not be
     * uniform; the default sampling is cosine-weighted sampling of the unit
     * hemisphere on the same side of the normal as the incoming vector (i.e.
     * reflection, not transmission).
     *
     * Override this function to change this behavior, e.g. to use a BTDF or to
     * use a BRDF with a delta distribution.
     *
     * @param rng               the per-thread RNG in use
     * @param incoming    [out] the sampled direction; the pointer must not be
     *                          null
     * @param outgoingOut [out] the probability of the sample, between 0 and 1;
     *                          the pointer must not be null
     * @returns                 the value of the BSDF at the incoming/outgoing
     *                          direction combination
     */
    virtual Vec sampleBSDF(
      Randomness& rng,
      const Vec& incoming,
      Vec* outgoingOut,
      float* probabilityOut
    ) const;

    /**
     * Evaluates the BSDF for an incoming and an outgoing direction. If you have
     * not changed the default behavior of BSDF::sampleBSDF, you can assume that
     * the incoming and outgoing directions are in the same hemisphere.
     * 
     * Implement this function to specify the BSDF used for material rendering.
     * You should return 0 for a function with a delta distribution, since the
     * probability of having exactly matching incoming and outgoing directions
     * would be 0.
     */
    virtual Vec evalBSDF(
      const Vec& incoming,
      const Vec& outgoing
    ) const = 0;

  public:
    virtual LightRay propagate(
      const LightRay& incoming,
      const Intersection& isect,
      Randomness& rng
    ) const;
  };

}
