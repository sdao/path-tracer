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
  Vec randOnLight = emissionObj->samplePoint(rng);
  Vec outgoing = (randOnLight - isect.position).normalized();
  Ray pointToLight(isect.position + outgoing * math::VERY_SMALL, outgoing);

  // Send a shadow ray.
  Intersection lightIsect;
  if (emissionObj != kdt.intersect(pointToLight, &lightIsect)) {
    // No intersection at all with this light.
    return Vec(0, 0, 0);
  } else if (!math::isNearlyZero(lightIsect.position - randOnLight)) {
    // The first intersection is not the random sampled point.
    return Vec(0, 0, 0);
  }

  float prob = (lightIsect.distance * lightIsect.distance)
    / (lightIsect.normal.dot(-outgoing) * emissionObj->area());
  Vec bsdf = mat->evalBSDFWorld(isect, -incoming.direction, outgoing);

  return bsdf.cwiseProduct(color) * (isect.normal.dot(outgoing)) / prob;
}
