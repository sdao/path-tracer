#include "material.h"

Material::~Material() {}

LightRay Material::scatter(
  Randomness& rng,
  const LightRay& incoming,
  const Intersection& isect
) const {
  Vec outgoingWorld;
  Vec bsdf;
  float pdf;
  sampleWorld(isect, rng, -incoming.direction, &outgoingWorld, &bsdf, &pdf);

  Vec scale;
  if (pdf > 0.0f) {
    scale = bsdf * fabsf(isect.normal.dot(outgoingWorld)) / pdf;
  } else {
    scale = Vec(0, 0, 0);
  }

  return LightRay(
    isect.position + outgoingWorld * math::VERY_SMALL,
    outgoingWorld,
    incoming.color.cwiseProduct(scale)
  );
}

float Material::evalPDFLocal(const Vec& incoming, const Vec& outgoing) const {
  if (!math::localSameHemisphere(incoming, outgoing)) {
    return 0.0f;
  }

  return math::cosineSampleHemispherePDF(outgoing);
}

void Material::evalLocal(
  const Vec& incoming,
  const Vec& outgoing,
  Vec* bsdfOut,
  float* pdfOut
) const {
  *bsdfOut = evalBSDFLocal(incoming, outgoing);
  *pdfOut = evalPDFLocal(incoming, outgoing);
}

void Material::evalWorld(
  const Intersection& isect,
  const Vec& incoming,
  const Vec& outgoing,
  Vec* bsdfOut,
  float* pdfOut
) const {
  Vec tangent;
  Vec binormal;
  math::coordSystem(isect.normal, &tangent, &binormal);

  // BSDF and PDF computation expects rays to be in local-space.
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

  evalLocal(incomingLocal, outgoingLocal, bsdfOut, pdfOut);
}

void Material::sampleLocal(
  Randomness& rng,
  const Vec& incoming,
  Vec* outgoingOut,
  Vec* bsdfOut,
  float* pdfOut
) const {
  Vec outgoing = math::cosineSampleHemisphere(rng, incoming.z() < 0.0f);

  *outgoingOut = outgoing;
  *bsdfOut = evalBSDFLocal(incoming, outgoing);
  *pdfOut = math::cosineSampleHemispherePDF(outgoing);
}

void Material::sampleWorld(
  const Intersection& isect,
  Randomness& rng,
  const Vec& incoming,
  Vec* outgoingOut,
  Vec* bsdfOut,
  float* pdfOut
) const {
  Vec tangent;
  Vec binormal;
  math::coordSystem(isect.normal, &tangent, &binormal);

  // BSDF computation expects incoming ray to be in local-space.
  Vec incomingLocal = math::worldToLocal(
    incoming,
    tangent,
    binormal,
    isect.normal
  );

  // Sample BSDF for direction, color, and probability.
  Vec outgoingLocal;
  Vec tempBsdf;
  float tempPdf;
  sampleLocal(rng, incomingLocal, &outgoingLocal, &tempBsdf, &tempPdf);

  // Rendering expects outgoing ray to be in world-space.
  Vec outgoingWorld = math::localToWorld(
    outgoingLocal,
    tangent,
    binormal,
    isect.normal
  );

  *outgoingOut = outgoingWorld;
  *bsdfOut = tempBsdf;
  *pdfOut = tempPdf;
}
