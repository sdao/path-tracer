#include "emitter.h"

materials::Emitter::Emitter(Vec c) : color(c) {}

LightRay materials::Emitter::propagate(
  const LightRay& incoming,
  const Intersection& /* isect */,
  Randomness& /* rng */
) const {
  return LightRay(
    Vec(0, 0, 0),
    Vec(0, 0, 0),
    incoming.color.cwiseProduct(color)
  );
}
