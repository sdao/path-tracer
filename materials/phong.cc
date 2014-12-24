#include "phong.h"

materials::phong::phong(float e, vec c)
  : exponent(e), color(c),
    scaleBRDF(c * (e + 2.0f) / math::TWO_PI),
    scaleProb((e + 1.0f) / math::TWO_PI),
    invExponent(1.0f / (e + 1.0f)) {}

vec materials::phong::evalBSDF(
  const vec& incoming,
  const vec& outgoing
) const {
  // See Lafortune & Willems <http://www.graphics.cornell.edu/~eric/Phong.html>.
  vec perfectReflect(-incoming.x(), -incoming.y(), incoming.z());
  float cosAlpha = std::pow(outgoing.dot(perfectReflect), exponent);

  return scaleBRDF * cosAlpha;
}

vec materials::phong::sampleBSDF(
  randomness& rng,
  const vec& incoming,
  vec* outgoingOut,
  float* probabilityOut
) const {
  // See Lafortune & Willems <http://www.graphics.cornell.edu/~eric/Phong.html>
  // for a derivation of the sampling procedure and PDF.
  vec perfectReflect(-incoming.x(), -incoming.y(), incoming.z());
  vec reflectTangent;
  vec reflectBinormal;

  math::coordSystem(perfectReflect, &reflectTangent, &reflectBinormal);

  /* The below procedure should produce unit vectors.
   *
   * Verify using this Mathematica code:
   * @code
   * R[a_] := (invExponent = 1/(20+1);
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
  vec local(cosf(phi) * sinTheta, sinf(phi) * sinTheta, cosTheta);

  *outgoingOut = math::localToWorld(
    local,
    reflectTangent,
    reflectBinormal,
    perfectReflect
  );
  float cosAlpha = std::pow(outgoingOut->dot(perfectReflect), exponent);
  *probabilityOut = scaleProb * cosAlpha;

  // Calculate the BRDF here instead of calling evalBSDF() since we already
  // have all of the info needed.
  return scaleBRDF * cosAlpha;
}
