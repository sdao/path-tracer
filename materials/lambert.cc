#include "lambert.h"

materials::lambert::lambert(vec a) : albedo(a) {}

vec materials::lambert::evalBSDF(
  const vec& incoming,
  const vec& outgoing
) const {
  return albedo * math::INV_PI;
}
