#include "phong.h"

materials::phong::phong(float e) : invExponent(1.0f / e) {}

lightray materials::phong::propagate(
  const lightray& incoming,
  const intersection& isect,
  randomness& rng
) const {
  // See <http://http.developer.nvidia.com/GPUGems3/gpugems3_ch20.html>.
  // I tested it in Mathematica, and it seems to work OK!
  vec reflectVector = math::reflect(incoming.direction, isect.normal);
  vec reflectTangent;
  vec reflectBinormal;
  math::coordSystem(reflectVector, &reflectTangent, &reflectBinormal);

  float cosTheta = std::pow(rng.nextUnitFloat(), invExponent);
  float sinTheta = sqrtf(1.0f - cosTheta * cosTheta);
  float phi = math::TWO_PI * rng.nextUnitFloat();

  vec phongVector =
    cosf(phi) * sinTheta * reflectTangent
    + cosTheta * reflectVector
    + sinf(phi) * sinTheta * reflectBinormal;

  return lightray(
    isect.position + phongVector * math::VERY_SMALL,
    phongVector,
    incoming.color
  );
}
