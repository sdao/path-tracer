#include "phong.h"

materials::Phong::Phong(float e, Vec c)
  : scaleBRDF(c * (e + 2.0f) / math::TWO_PI),
    scaleProb((e + 1.0f) / math::TWO_PI),
    invExponent(1.0f / (e + 1.0f)),
    exponent(e), color(c) {}

Vec materials::Phong::evalBSDF(
  const Vec& incoming,
  const Vec& outgoing
) const {
  // See Lafortune & Willems <http://www.graphics.cornell.edu/~eric/Phong.html>.
  Vec perfectReflect(-incoming.x(), -incoming.y(), incoming.z());
  float cosAlpha = std::pow(outgoing.dot(perfectReflect), exponent);

  return scaleBRDF * cosAlpha;
}

Vec materials::Phong::sampleBSDF(
  Randomness& rng,
  const Vec& incoming,
  Vec* outgoingOut,
  float* probabilityOut
) const {
  // See Lafortune & Willems <http://www.graphics.cornell.edu/~eric/Phong.html>
  // for a derivation of the sampling procedure and PDF.
  Vec perfectReflect(-incoming.x(), -incoming.y(), incoming.z());
  Vec reflectTangent;
  Vec reflectBinormal;

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
  Vec local(cosf(phi) * sinTheta, sinf(phi) * sinTheta, cosTheta);

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
