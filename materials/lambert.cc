#include "lambert.h"

materials::Lambert::Lambert(Vec a) : albedo(a) {}

Vec materials::Lambert::evalBSDF(
  const Vec& /* incoming */,
  const Vec& /* outgoing */
) const {
  return albedo * math::INV_PI;
}
