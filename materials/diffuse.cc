#include "diffuse.h"

materials::diffuse::diffuse(vec c) : material(), color(c) {}

lightray materials::diffuse::propagate(
  const lightray& incoming,
  intersection& isect,
  randomness& rng
) const {
  vec reflectVector = isect.uniformSampleHemisphere(rng);

  return lightray(
    isect.position + reflectVector * math::VERY_SMALL,
    reflectVector,
    incoming.color * color
  );
}
