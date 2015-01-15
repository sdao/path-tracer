#include "phong.h"

using std::min;
using std::max;

materials::Phong::Phong(float e, const Vec& c)
  : scaleBRDF(c * (e + 2.0f) / math::TWO_PI),
    scaleProb((e + 1.0f) / math::TWO_PI),
    invExponent(1.0f / (e + 1.0f)),
    color(c), exponent(e) {}

materials::Phong::Phong(const Node& n)
  : Phong(n.getFloat("exponent"), n.getVec("color")) {}

inline Vec materials::Phong::evalBSDFInternal(
  const Vec& perfectReflect,
  const Vec& outgoing
) const {
  float cosAlpha = max(0.0f, outgoing.dot(perfectReflect));
  float cosAlphaPow = std::pow(cosAlpha, exponent);

  return scaleBRDF * cosAlphaPow;
}

inline float materials::Phong::evalPDFInternal(
  const Vec& perfectReflect,
  const Vec& outgoing
) const {
  float cosAlpha = max(0.0f, outgoing.dot(perfectReflect));
  float cosAlphaPow = std::pow(cosAlpha, exponent);

  return scaleProb * cosAlphaPow;
}

Vec materials::Phong::evalBSDFLocal(
  const Vec& incoming,
  const Vec& outgoing
) const {
  // See Lafortune & Willems <http://www.graphics.cornell.edu/~eric/Phong.html>.
  if (!math::localSameHemisphere(incoming, outgoing)) {
    return Vec(0, 0, 0);
  }

  Vec perfectReflect(-incoming.x(), -incoming.y(), incoming.z());
  return evalBSDFInternal(perfectReflect, outgoing);
}

float materials::Phong::evalPDFLocal(
  const Vec& incoming,
  const Vec& outgoing
) const {
  if (!math::localSameHemisphere(incoming, outgoing)) {
    return 0.0f;
  }

  Vec perfectReflect(-incoming.x(), -incoming.y(), incoming.z());
  return evalPDFInternal(perfectReflect, outgoing);
}

void materials::Phong::sampleLocal(
  Randomness& rng,
  const Vec& incoming,
  Vec* outgoingOut,
  Vec* bsdfOut,
  float* pdfOut
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

  // Here, "local" being the space of the perfect reflection vector and
  // "world" being the space of the normal.
  *outgoingOut = math::localToWorld(
    local,
    reflectTangent,
    reflectBinormal,
    perfectReflect
  );
  *bsdfOut = evalBSDFInternal(perfectReflect, *outgoingOut);
  *pdfOut = evalPDFInternal(perfectReflect, *outgoingOut);
}

bool materials::Phong::shouldDirectIlluminate() const {
  return true;
}
