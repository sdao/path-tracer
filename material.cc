#include "material.h"

Material::~Material() {}

LightRay Material::propagate(
  const LightRay& incoming,
  const Intersection& isect,
  Randomness& rng
) const {
  Vec tangent;
  Vec binormal;
  math::coordSystem(isect.normal, &tangent, &binormal);

  // BRDF computation expects incoming ray to be in local-space.
  Vec incomingLocal = math::worldToLocal(
    -incoming.direction,
    tangent,
    binormal,
    isect.normal
  );

  // Sample BSDF for direction, color, and probability.
  Vec outgoingLocal;
  float probOutgoing;
  Vec brdf = sampleBSDF(rng, incomingLocal, &outgoingLocal, &probOutgoing);
  Vec scale = brdf * math::absCosTheta(outgoingLocal) / probOutgoing;

  // Rendering expects outgoing ray to be in world-space.
  Vec outgoingWorld = math::localToWorld(
    outgoingLocal,
    tangent,
    binormal,
    isect.normal
  );

  return LightRay(
    isect.position + outgoingWorld * math::VERY_SMALL,
    outgoingWorld,
    incoming.color.cwiseProduct(scale)
  );
}

Vec Material::sampleBSDF(
  Randomness& rng,
  const Vec& incoming,
  Vec* outgoingOut,
  float* probabilityOut
) const {
  math::cosineSampleHemisphere(rng, outgoingOut, probabilityOut);

  // Generate the output direction on the same side of the normal as the
  // input direction (reflection) by default, i.e. assume BRDF.
  if (incoming[2] < 0.0f) {
    (*outgoingOut)[2] *= -1.0f;
  }

  return evalBSDF(incoming, *outgoingOut);
}

Vec Material::evalBSDF(
  const Vec& /* incoming */,
  const Vec& /* outgoing */
) const {
  return Vec(0, 0, 0);
}
