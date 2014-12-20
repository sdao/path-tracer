#include "glossy.h"

materials::glossy::glossy(float g) : glossiness(g) {}

lightray materials::glossy::propagate(
  const lightray& incoming,
  intersection& isect,
  randomness& rng
) const {
  vec reflectVector = isect.uniformSampleCone(rng, glossiness * float(M_PI_2));

  return lightray(
    isect.position + reflectVector * math::VERY_SMALL,
    reflectVector,
    incoming.color
  );
}
