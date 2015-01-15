#include "lambert.h"

materials::Lambert::Lambert(const Vec& a) : albedo(a) {}

materials::Lambert::Lambert(const Node& n) : Lambert(n.getVec("albedo")) {}

Vec materials::Lambert::evalBSDFLocal(
  const Vec& incoming,
  const Vec& outgoing
) const {
  if (!math::localSameHemisphere(incoming, outgoing)) {
    return Vec(0, 0, 0);
  }

  return albedo * math::INV_PI;
}

bool materials::Lambert::shouldDirectIlluminate() const {
  return true;
}
