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

  /* The below procedure should produce unit vectors.
   *
   * Verify using this Mathematica code:
   * @code
   * R[a_] := (invExponent = 1/20;
   *   cosTheta = RandomReal[{0, 1}]^invExponent;
   *   sinTheta = Sqrt[1 - cosTheta*cosTheta];
   *   phi = 2*Pi*RandomReal[{0, 1}];
   *   {Cos[phi]*sinTheta, cosTheta, Sin[phi]*sinTheta})
   *
   * LenR[a_] := Norm[R[a]]
   * 
   * ListPointPlot3D[Map[R, Range[1000]], BoxRatios -> Automatic]
   *
   * LenR /@ Range[1000]
   *
   * @endcode
   */
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
