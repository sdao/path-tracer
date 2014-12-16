#include "emitter.h"
#include <memory>

materials::emitter::emitter(vec c) : material(), color(c) {}

lightray materials::emitter::propagate(
  const lightray& incoming,
  intersection& /* isect */,
  randomness& /* rng */
) const {
  return lightray(vec(0), vec(0), incoming.color * color);
}

materialptr materials::emitter::make(vec c) {
  return std::make_shared<emitter>(c);
}
