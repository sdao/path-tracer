#include "fresnel.h"

materials::fresnel::fresnel(float ior)
  : etaEntering(IOR_VACUUM / ior), etaExiting(ior / IOR_VACUUM) {
  // Pre-compute values for Fresnel calculations.

  float r0_temp = (IOR_VACUUM - ior) / (IOR_VACUUM + ior);
  r0 = r0_temp * r0_temp;
}

lightray materials::fresnel::propagate(
  const lightray& incoming,
  const intersection& isect,
  randomness& rng
) const {
  // Entering = are normal and ray in opposite directions?
  bool entering = isect.normal.dot(incoming.direction) < 0;

  vec alignedNormal; // Normal flipped based on ray direction.
  float eta; // Ratio of indices of refraction.
  if (entering) {
    // Note: geometry will return surface normal pointing outwards.
    // If we are entering, this is the right normal.
    // If we are exiting, since geometry is single-shelled, we will need
    // to flip the normal.
    alignedNormal = isect.normal;
    eta = etaEntering;
  } else {
    alignedNormal = -isect.normal;
    eta = etaExiting;
  }

  // Calculate reflection vector.
  vec reflectVector = math::reflect(
    incoming.direction,
    alignedNormal
  );

  // Calculate refraction vector.
  vec refractVector = math::refract(
    incoming.direction,
    alignedNormal,
    eta
  );

  if (math::isNearlyZero(refractVector.squaredNorm())) {
    // Total internal reflection. Must reflect.
    return lightray(
      isect.position + reflectVector * math::VERY_SMALL,
      reflectVector,
      incoming.color
    );
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
    cosTemp = 1.0f - (-incoming.direction).dot(alignedNormal);
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
    return lightray(
      isect.position + reflectVector * math::VERY_SMALL,
      reflectVector,
      incoming.color * refl / probRefl // Adjust color for probabilities.
    );
  } else {
    return lightray(
      isect.position + refractVector * math::VERY_SMALL,
      refractVector,
      incoming.color * refr / probRefr // Adjust color for probabilities.
    );
  }
}
