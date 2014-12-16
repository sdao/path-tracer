#include "glossy.h"
#include <memory>

materials::glossy::glossy(float g) : material(), glossiness(g) {}

lightray materials::glossy::propagate(
  const lightray& incoming,
  intersection& isect,
  randomness& rng
) const {
  vec reflectVector = isect.uniformSampleCone(rng, glossiness * float(M_PI_2));

  return lightray(
    isect.position + reflectVector * 0.01f,
    reflectVector,
    incoming.color
  );
}

materialptr materials::glossy::make(float g) {
  return std::make_shared<glossy>(g);
}
