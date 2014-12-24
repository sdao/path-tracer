#include "diffuse.h"

materials::diffuse::diffuse(vec c) : color(c) {}

lightray materials::diffuse::propagate(
  const lightray& incoming,
  const intersection& isect,
  randomness& rng
) const {
  vec tangent;
  vec binormal;
  math::coordSystem(isect.normal, &tangent, &binormal);

  vec dir;
  float prob;
  math::cosineSampleHemisphere(rng, &dir, &prob);

  vec reflectVector = math::localToWorld(dir, tangent, binormal, isect.normal);

  return lightray(
    isect.position + reflectVector * math::VERY_SMALL,
    reflectVector,
    incoming.color.cwiseProduct(color)
  );
}
