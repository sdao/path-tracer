#pragma once
#include "core.h"

/**
 * A material that specifies how light scatters on geometry using a BSDF.
 */
class Material {
public:
  virtual ~Material();

  /**
   * Determines whether another ray should be cast as a consequence of a
   * lightray hitting a surface.
   *
   * @param incoming the incoming ray that struck the surface
   * @param isect    the intersection information for the incoming ray
   * @param rng      the per-thread RNG in use
   * @returns        a lightray to cast as a consequence; a zero-length ray will
   *                 terminate the path
   */
  LightRay scatter(
    Randomness& rng,
    const LightRay& incoming,
    const Intersection& isect
  ) const;

  /**
   * Samples the BSDF at a random output direction in the local (normal)
   * coordinate system. The sampling need not be uniform; the default sampling
   * is cosine-weighted sampling of the unit hemisphere on the same side of the
   * normal as the incoming vector (i.e. reflection, not transmission).
   *
   * @param rng               the per-thread RNG in use
   * @param incoming    [out] the sampled direction; the pointer must not be
   *                          null
   * @param outgoingOut [out] the probability of the sample, between 0 and 1;
   *                          the pointer must not be null
   * @returns                 the value of the BSDF at the incoming/outgoing
   *                          direction combination
   */
  virtual Vec sampleBSDFLocal(
    Randomness& rng,
    const Vec& incoming,
    Vec* outgoingOut,
    float* probabilityOut
  ) const;

  /**
   * Evaluates the BSDF for an incoming and an outgoing direction in the local
   * (normal) coordinate system. If you have not changed the default behavior of
   * BSDF::sampleBSDF, you can assume that the incoming and outgoing directions
   * are in the same hemisphere.
   * 
   * You should return 0 for a function with a delta distribution, since the
   * probability of having exactly matching incoming and outgoing directions
   * would be 0.
   */
  virtual Vec evalBSDFLocal(const Vec& incoming, const Vec& outgoing) const = 0;

  /**
   * Evaluates the BSDF for an incoming and an outgoing direction, given an
   * intersection frame.
   */
  Vec evalBSDFWorld(
    const Intersection& isect,
    const Vec& incoming,
    const Vec& outgoing
  ) const;

  /**
   * Returns true if direct illumination should be estimated for surfaces with
   * this material. Otherwise, only path tracing will be used.
   */
  virtual bool shouldDirectIlluminate() const = 0;
};
