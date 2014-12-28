#include "material.h"

Material::~Material() {}

LightRay Material::scatter(
  Randomness& rng,
  const LightRay& incoming,
  const Intersection& isect
) const {
  Vec tangent;
  Vec binormal;
  math::coordSystem(isect.normal, &tangent, &binormal);

  // BSDF computation expects incoming ray to be in local-space.
  Vec incomingLocal = math::worldToLocal(
    -incoming.direction,
    tangent,
    binormal,
    isect.normal
  );

  // Sample BSDF for direction, color, and probability.
  Vec outgoingLocal;
  float probOutgoing;
  Vec bsdf = sampleBSDFLocal(rng, incomingLocal, &outgoingLocal, &probOutgoing);
  Vec scale = bsdf * math::absCosTheta(outgoingLocal) / probOutgoing;

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

Vec Material::sampleBSDFLocal(
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

  return evalBSDFLocal(incoming, *outgoingOut);
}

Vec Material::evalBSDFWorld(
  const Intersection& isect,
  const Vec& incoming,
  const Vec& outgoing
) const {
  Vec tangent;
  Vec binormal;
  math::coordSystem(isect.normal, &tangent, &binormal);

  // BSDF computation expects rays to be in local-space.
  Vec incomingLocal = math::worldToLocal(
    incoming,
    tangent,
    binormal,
    isect.normal
  );

  Vec outgoingLocal = math::worldToLocal(
    outgoing,
    tangent,
    binormal,
    isect.normal
  );

  return evalBSDFLocal(incomingLocal, outgoingLocal);
}
