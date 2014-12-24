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
    -incoming.direction,
    tangent,
    binormal,
    isect.normal
  );

  // Sample BSDF for direction, color, and probability.
  vec outgoingLocal;
  float probOutgoing;
  vec brdf = sampleBSDF(rng, incomingLocal, &outgoingLocal, &probOutgoing);
  vec scale = brdf * math::absCosTheta(outgoingLocal) / probOutgoing;

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

vec material::sampleBSDF(
  randomness& rng,
  const vec& incoming,
  vec* outgoingOut,
  float* probabilityOut
) const {
  math::cosineSampleHemisphere(rng, outgoingOut, probabilityOut);

  // Generate the output direction on the same side of the normal as the
  // input direction (reflection) by default, i.e. assume BRDF.
  if (incoming[2] < 0.0f) {
    outgoingOut[2] *= -1.0f;
  }

  return evalBSDF(incoming, *outgoingOut);
}

vec material::evalBSDF(
  const vec& incoming,
  const vec& outgoing
) const {
  return vec(0, 0, 0);
}
