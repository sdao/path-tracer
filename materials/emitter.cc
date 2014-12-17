#include "emitter.h"

materials::emitter::emitter(vec c) : material(), color(c) {}

lightray materials::emitter::propagate(
  const lightray& incoming,
  intersection& /* isect */,
  randomness& /* rng */
) const {
  return lightray(vec(0), vec(0), incoming.color * color);
}
