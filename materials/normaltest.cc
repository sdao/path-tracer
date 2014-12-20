#include "normaltest.h"

materials::normaltest::normaltest() {}

lightray materials::normaltest::propagate(
  const lightray& /* incoming */,
  const intersection& isect,
  randomness& /* rng */
) const {
  return lightray(vec(0, 0, 0), vec(0, 0, 0), isect.normal);
}
