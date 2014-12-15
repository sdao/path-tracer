#include "fresnel.h"
#include <memory>

materials::fresnel::fresnel(float ior) : material(), idxRefract(ior) {}

lightray materials::fresnel::propagate(
  const lightray& incoming,
  intersection& isect,
  randomness& rng
) const {
  // Entering = are normal and ray in opposite directions?
  bool entering = glm::dot(isect.normal, incoming.direction) < 0;
  vec alignedNormal;
  float nIncident, nTransmit; // Indices of refraction.
  if (entering) {
    // Note: geometry will return surface normal pointing outwards.
    // If we are entering, this is the right normal.
    // If we are exiting, since geometry is single-shelled, we will need
    // to flip the normal.
    alignedNormal = isect.normal;
    nIncident = IOR_VACUUM;
    nTransmit = idxRefract;
  } else {
    alignedNormal = -isect.normal;
    nIncident = idxRefract;
    nTransmit = IOR_VACUUM;
  }

  // Calculate reflection vector using GLM.
  vec reflectVector = glm::reflect(
    incoming.unit().direction,
    alignedNormal
  );

  // Calculate refraction vector using GLM.
  vec refractVector = glm::refract(
    incoming.unit().direction,
    alignedNormal,
    nIncident / nTransmit
  );
  if (math::isNearlyZero(glm::length(refractVector))) {
    // Total internal reflection. Must reflect.
    return lightray(
      isect.position + reflectVector * 0.01f,
      reflectVector,
      incoming.color
    );
  }

  // Calculates Fresnel reflectance factor using Schlick's approximation.
  // See <http://graphics.stanford.edu/
  // courses/cs148-10-summer/docs/2006--degreve--reflection_refraction.pdf>.
  float r0_temp = (nIncident - nTransmit) / (nIncident + nTransmit);
  float r0 = r0_temp * r0_temp;
  float cosTemp;
  if (nIncident < nTransmit) {
    // Equivalent to condition: entering == true
    // (e.g. nI = 1 (air), nT = 1.5 (glass))
    // Theta = angle of incidence.
    cosTemp = 1.0f - glm::dot(-incoming.unit().direction, alignedNormal);
  } else {
    // Equivalent to condition: entering == false
    // Theta = angle of refraction.
    cosTemp = 1.0f - glm::dot(refractVector, -alignedNormal);
  }

  float cosTemp_5 = cosTemp * cosTemp * cosTemp * cosTemp * cosTemp;
  float refl = r0 + (1.0f - r0) * cosTemp_5;
  float refr = 1.0f - refl;

  // Importance sampling probabilities.
  // Pr[cast reflect ray] = [0.25, 0.75] based on reflectance.
  float probRefl = 0.25f + 0.5f * refl;
  float probRefr = 1.0f - probRefl;

  // Probabilistically choose to refract or reflect.
  if (rng.nextUnitFloat() < probRefl) {
    // Higher reflectance = higher probability of reflecting.
    return lightray(
      isect.position + reflectVector * 0.01f,
      reflectVector,
      incoming.color * refl / probRefl // Adjust color for probabilities.
    );
  } else {
    return lightray(
      isect.position + refractVector * 0.01f,
      refractVector,
      incoming.color * refr / probRefr // Adjust color for probabilities.
    );
  }
}

materialptr materials::fresnel::make(float ior) {
  return std::make_shared<fresnel>(ior);
}
