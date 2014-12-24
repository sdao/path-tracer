#pragma once
#include "core.h"

/**
 * A material that specified how light interacts with geometry.
 */
class material {
protected:
  /**
   * Samples the BSDF at a random output direction. The sampling need not be
   * uniform; the default sampling is cosine-weighted.
   *
   * @param rng               the per-thread RNG in use
   * @param incoming    [out] the sampled direction; the pointer must not be
   *                          null
   * @param outgoingOut [out] the probability of the sample, between 0 and 1;
   *                          the pointer must not be null
   * @returns                 the value of the BSDF at the incoming/outgoing
   *                          direction combination
   */
  virtual vec sampleBSDF(
    randomness& rng,
    const vec& incoming,
    vec* outgoingOut,
    float* probabilityOut
  ) const;

  /**
   * Evaluates the BSDF for an incoming and an outgoing direction.
   */
  virtual vec evalBSDF(
    const vec& incoming,
    const vec& outgoing
  ) const;

public:
  virtual ~material();

  /**
   * Determines whether another ray should be cast as a consequence of a
   * lightray hitting a surface.
   *
   * Override this function to replace the default BSDF evaluation system for
   * materials.
   *
   * @param incoming the incoming ray that struck the surface
   * @param isect    the intersection information for the incoming ray
   * @param rng      the per-thread RNG in use
   * @returns        a lightray to cast as a consequence; use a colored
   *                 zero-length ray to indicate that we have reached an emitter
   *                 and a black zero-length ray to indicate that the path gives
   *                 no light
   */
  virtual lightray propagate(
    const lightray& incoming,
    const intersection& isect,
    randomness& rng
  ) const;
};
