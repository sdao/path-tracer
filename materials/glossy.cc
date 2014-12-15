#include "glossy.h"
#include <memory>

materials::glossy::glossy(float g) : glossiness(g), material() {}

lightray materials::glossy::propagate(
  const lightray& incoming,
  intersection& isect,
  randomness& rng
) const {
  vec reflectVector = isect.uniformSampleHemisphere(rng);

  return lightray(
    isect.position + reflectVector * 0.01f,
    glossiness * reflectVector + (1.0f - glossiness) * isect.normal,
    incoming.color
  );
}

materialptr materials::glossy::make(float g) {
  return std::make_shared<glossy>(g);
}
