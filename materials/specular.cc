#include "specular.h"
#include <memory>

materials::specular::specular() : material() {}

lightray materials::specular::propagate(
  const lightray& incoming,
  intersection& isect,
  randomness& /* rng */
) const {
  vec reflectVector = glm::reflect(incoming.direction, isect.normal);
  
  return lightray(
    isect.position + reflectVector * 0.01f,
    reflectVector,
    incoming.color
  );
}

materialptr materials::specular::make() {
  return std::shared_ptr<specular>();
}
