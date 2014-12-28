#include "light.h"

AreaLight::AreaLight(Vec c) : color(c) {}

Vec AreaLight::directIlluminate(
  Randomness& rng,
  const Ray& incoming,
  const Intersection& isect,
  const Material* mat,
  const Geom* emissionObj,
  const KDTree& kdt
) const {
  Vec lightPosition;
  Vec lightNormal;
  emissionObj->samplePoint(rng, &lightPosition, &lightNormal);

  Vec outgoing = (lightPosition - isect.position).normalized();
  Ray pointToLight(isect.position + outgoing * math::VERY_SMALL, outgoing);
  float distToLight2 = (lightPosition - isect.position).squaredNorm();

  // Why do we subtract 2 * VERY_SMALL?
  // 1. To balance out the added VERY_SMALL in the ray pointToLight, and
  // 2. Because we don't want to actually hit the light itself!
  float shadowDist = sqrtf(distToLight2) - 2.0f * math::VERY_SMALL;

  // Send a shadow ray.
  if (kdt.intersectShadow(pointToLight, shadowDist)) {
    // Uh-oh, something intersected before the ray hit the light!
    return Vec(0, 0, 0);
  }

  // Calculate BSDF and probability of hitting light.
  float prob =
    distToLight2 / (lightNormal.dot(-outgoing) * emissionObj->area());
  Vec bsdf = mat->evalBSDFWorld(isect, -incoming.direction, outgoing);

  return bsdf.cwiseProduct(color) * (isect.normal.dot(outgoing)) / prob;
}
