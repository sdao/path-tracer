#include "material.h"

material::~material() {}

lightray material::propagate(
  const lightray& incoming,
  const intersection& isect,
  randomness& rng
) const {
  vec tangent;
  vec binormal;
  math::coordSystem(isect.normal, &tangent, &binormal);

  // BRDF computation expects incoming ray to be in local-space.
  vec incomingLocal = math::worldToLocal(
    incoming.direction,
    tangent,
    binormal,
    isect.normal
  );

  // Use hemisphere sampling for default.
  vec outgoingLocal;
  float probOutgoing;
  sampleDirection(rng, &outgoingLocal, &probOutgoing);

  // Compute BRDF value and final scale factor.
  vec brdf = evalBRDF(incomingLocal, outgoingLocal);
  vec scale = brdf * math::cosTheta(outgoingLocal) / probOutgoing;

  // Rendering expects outgoing ray to be in world-space.
  vec outgoingWorld = math::localToWorld(
    outgoingLocal,
    tangent,
    binormal,
    isect.normal
  );

  return lightray(
    isect.position + outgoingWorld * math::VERY_SMALL,
    outgoingWorld,
    incoming.color.cwiseProduct(scale)
  );
}

void material::sampleDirection(
  randomness& rng,
  vec* directionOut,
  float* probabilityOut
) const {
  math::cosineSampleHemisphere(rng, directionOut, probabilityOut);
}

vec material::evalBRDF(
  const vec& incoming,
  const vec& outgoing
) const {
  return vec(0, 0, 0);
}
