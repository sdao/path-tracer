#include "dielectric.h"

materials::Dielectric::Dielectric(float ior, Vec c)
  : etaEntering(IOR_VACUUM / ior), etaExiting(ior / IOR_VACUUM), color(c) {
  // Pre-compute values for Fresnel calculations.

  float r0_temp = (IOR_VACUUM - ior) / (IOR_VACUUM + ior);
  r0 = r0_temp * r0_temp;
}

Vec materials::Dielectric::evalBSDF(
  const Vec& /* incoming */,
  const Vec& /* outgoing */
) const {
  // Probabilistically, we are never going to get the exact matching
  // incoming and outgoing vectors.
  return Vec(0, 0, 0);
}

Vec materials::Dielectric::sampleBSDF(
  Randomness& rng,
  const Vec& incoming,
  Vec* outgoingOut,
  float* probabilityOut
) const {
  // Entering = are normal and incoming direction in opposite directions?
  // Recall that the incoming direction is in the normal's local space.
  bool entering = incoming.z() > 0.0f;

  Vec alignedNormal; // Normal flipped based on ray direction.
  float eta; // Ratio of indices of refraction.
  if (entering) {
    // Note: geometry will return surface normal pointing outwards.
    // If we are entering, this is the right normal.
    // If we are exiting, since geometry is single-shelled, we will need
    // to flip the normal.
    alignedNormal = Vec(0, 0, 1);
    eta = etaEntering;
  } else {
    alignedNormal = Vec(0, 0, -1);
    eta = etaExiting;
  }

  // Calculate reflection vector.
  Vec reflectVector = math::reflect(
    -incoming,
    alignedNormal
  );

  // Calculate refraction vector.
  Vec refractVector = math::refract(
    -incoming,
    alignedNormal,
    eta
  );

  if (math::isNearlyZero(refractVector.squaredNorm())) {
    // Total internal reflection. Must reflect.
    *outgoingOut = reflectVector;
    *probabilityOut = 1.0f;
    return color / math::absCosTheta(reflectVector);
  }

  // Calculates Fresnel reflectance factor using Schlick's approximation.
  // See <http://graphics.stanford.edu/
  // courses/cs148-10-summer/docs/2006--degreve--reflection_refraction.pdf>.
  float cosTemp;
  if (eta < 1.0f) {
    // Equivalent to nIncident < nTransmit.
    // Equivalent to condition: entering == true
    // (e.g. nI = 1 (air), nT = 1.5 (glass))
    // Theta = angle of incidence.
    cosTemp = 1.0f - incoming.dot(alignedNormal);
  } else {
    // Equivalent to condition: entering == false
    // Theta = angle of refraction.
    cosTemp = 1.0f - refractVector.dot(-alignedNormal);
  }

  float cosTemp5 = cosTemp * cosTemp * cosTemp * cosTemp * cosTemp;
  float refl = r0 + (1.0f - r0) * cosTemp5;
  float refr = 1.0f - refl;

  // Importance sampling probabilities.
  // Pr[cast reflect ray] = [0.25, 0.75] based on reflectance.
  float probRefl = 0.25f + 0.5f * refl;
  float probRefr = 1.0f - probRefl;

  // Probabilistically choose to refract or reflect.
  if (rng.nextUnitFloat() < probRefl) {
    // Higher reflectance = higher probability of reflecting.
    *outgoingOut = reflectVector;
    *probabilityOut = probRefl;
    return color * refl / math::absCosTheta(reflectVector);
  } else {
    *outgoingOut = refractVector;
    *probabilityOut = probRefr;
    return color * refr / math::absCosTheta(refractVector);
  }
}