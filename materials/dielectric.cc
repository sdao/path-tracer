#include "dielectric.h"

materials::Dielectric::Dielectric(float ior, const Vec& c)
  : r0(schickR0(ior)),
    etaEntering(IOR_VACUUM / ior), etaExiting(ior / IOR_VACUUM),
    color(c) {}

materials::Dielectric::Dielectric(const Node& n)
  : Dielectric(n.getFloat("ior"), n.getVec("color")) {}

float materials::Dielectric::schickR0(float ior) {
  // Pre-compute values for Fresnel calculations.
  
  float r0_temp = (IOR_VACUUM - ior) / (IOR_VACUUM + ior);
  return r0_temp * r0_temp;
}

Vec materials::Dielectric::evalBSDFLocal(
  const Vec& /* incoming */,
  const Vec& /* outgoing */
) const {
  // Probabilistically, we are never going to get the exact matching
  // incoming and outgoing vectors.
  return Vec(0, 0, 0);
}

float materials::Dielectric::evalPDFLocal(
  const Vec& /* incoming */,
  const Vec& /* outgoing */
) const {
  // Probabilistically, we are never going to get the exact matching
  // incoming and outgoing vectors.
  return 0.0f;
}

void materials::Dielectric::sampleLocal(
  Randomness& rng,
  const Vec& incoming,
  Vec* outgoingOut,
  Vec* bsdfOut,
  float* pdfOut
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
    *bsdfOut = color / math::absCosTheta(reflectVector);
    *pdfOut = 1.0f;
    return;
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
  float probRefl = math::lerp(0.25f, 0.75f, refl);
  float probRefr = 1.0f - probRefl;

  // Probabilistically choose to refract or reflect.
  if (rng.nextUnitFloat() < probRefl) {
    // Higher reflectance = higher probability of reflecting.
    *outgoingOut = reflectVector;
    *bsdfOut = color * refl / math::absCosTheta(reflectVector);
    *pdfOut = probRefl;
  } else {
    *outgoingOut = refractVector;
    *bsdfOut = color * refr / math::absCosTheta(refractVector);
    *pdfOut = probRefr;
  }
}

bool materials::Dielectric::shouldDirectIlluminate() const {
  return false;
}
