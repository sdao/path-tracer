#include "specular.h"

materials::specular::specular() : material() {}

lightray materials::specular::propagate(
  const lightray& incoming,
  intersection& isect,
  randomness& /* rng */
) const {
  vec reflectVector = glm::reflect(incoming.direction, isect.normal);

  return lightray(
    isect.position + reflectVector * math::VERY_SMALL,
    reflectVector,
    incoming.color
  );
}
