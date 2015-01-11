#pragma once
#include "core.h"
#include "node.h"

/**
 * A material that specifies how light scatters on geometry using a BSDF.
 */
class Material {
protected:
  /**
   * Evaluates the BSDF for an incoming and an outgoing direction in the local
   * (normal) coordinate system. If you have not changed the default behavior of
   * Material::sampleBSDF, you can assume that the incoming and outgoing
   * directions are in the same hemisphere.
   * 
   * If the BSDF is a delta distribution (i.e. it is only non-zero for one
   * point), you should return 0 from this function and override
   * Material::sampleBSDFLocal only.
   */
  virtual Vec evalBSDFLocal(const Vec& incoming, const Vec& outgoing) const = 0;
  
  /**
   * Returns the probability that the given outgoing vector will be sampled
   * for the given incoming vector by the function Material::sampleBSDFLocal,
   * with both vectors in local space. The default implementation returns
   * the probability based on a cosine-weighted hemisphere.
   *
   * If you override this function, you should also override
   * Material::sampleBSDFLocal.
   */
  virtual float evalPDFLocal(const Vec& incoming, const Vec& outgoing) const;

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
   * Samples the BSDF and PDF at a random output direction in the local (normal)
   * coordinate system. The sampling need not be uniform; the default sampling
   * is cosine-weighted sampling of the unit hemisphere on the same side of the
   * normal as the incoming vector (i.e. reflection, not transmission).
   *
   * If you override this function, you must also override
   * Material::evalPDFLocal.
   *
   * @param rng               the per-thread RNG in use
   * @param incoming    [out] the sampled direction; the pointer must not be
   *                          null
   * @param outgoingOut [out] the outgoing vector that was sampled;
   *                          the pointer must not be null
   * @param bsdfOut     [out] the BSDF at the sample point;
   *                          the pointer must not be null
   * @param pdfOut      [out] the probability of the sample;
   *                          the pointer must not be null
   */
  virtual void sampleLocal(
    Randomness& rng,
    const Vec& incoming,
    Vec* outgoingOut,
    Vec* bsdfOut,
    float* pdfOut
  ) const;

  /**
   * Same as Material::sampleLocal, but returns the outgoing vector in world
   * space using the given intersection frame.
   */
  void sampleWorld(
    const Intersection& isect,
    Randomness& rng,
    const Vec& incoming,
    Vec* outgoingOut,
    Vec* bsdfOut,
    float* pdfOut
  ) const;

  /**
   * Evaluates the BSDF and PDF for an incoming and an outgoing direction in the
   * local (normal) coordinate system.
   *
   * @param incoming        the incoming direction in local space
   * @param outgoing        the outgoing direction in local space
   * @param bsdfOut  [out]  the value of the BSDF at the incoming/outgoing pair
   * @param pdfOut   [out]  the value of the PDF at the incoming/outgoing pair
   */
  void evalLocal(
    const Vec& incoming,
    const Vec& outgoing,
    Vec* bsdfOut,
    float* pdfOut
  ) const;

  /**
   * Same as Material::evalLocal, but requires the incoming and outcoming
   * vectors to be specified in world space using the given intersection frame.
   */
  void evalWorld(
    const Intersection& isect,
    const Vec& incoming,
    const Vec& outgoing,
    Vec* bsdfOut,
    float* pdfOut
  ) const;

  /**
   * Returns true if direct illumination should be estimated for surfaces with
   * this material. Otherwise, only path tracing will be used.
   */
  virtual bool shouldDirectIlluminate() const = 0;
};
