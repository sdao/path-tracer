#include "light.h"

AreaLight::AreaLight(Vec c) : color(c) {}

inline Vec AreaLight::directIlluminateByLightPDF(
  Randomness& rng,
  const Ray& incoming,
  const Intersection& isect,
  const Material* mat,
  const Geom* emissionObj,
  const KDTree& kdt
) const {
  // Sample random from light PDF.
  Vec outgoingWorld;
  Vec lightColor;
  float lightPdf;
  sampleLight(
    rng,
    emissionObj,
    kdt,
    isect.position,
    &outgoingWorld,
    &lightColor,
    &lightPdf
  );

  if (lightPdf > 0.0f && !math::isVectorExactlyZero(lightColor)) {
    // Evaluate material BSDF and PDF as well.
    Vec bsdf;
    float bsdfPdf;
    mat->evalWorld(
      isect,
      -incoming.direction,
      outgoingWorld,
      &bsdf,
      &bsdfPdf
    );

    float lightWeight = math::powerHeuristic(1, lightPdf, 1, bsdfPdf);
    return bsdf.cwiseProduct(lightColor)
      * fabsf(isect.normal.dot(outgoingWorld))
      * lightWeight / lightPdf;
  }

  return Vec(0, 0, 0);
}

inline Vec AreaLight::directIlluminateByMatPDF(
  Randomness& rng,
  const Ray& incoming,
  const Intersection& isect,
  const Material* mat,
  const Geom* emissionObj,
  const KDTree& kdt
) const {
  // Sample random from BSDF PDF.
  Vec outgoingWorld;
  Vec bsdf;
  float bsdfPdf;
  mat->sampleWorld(
    isect,
    rng,
    -incoming.direction,
    &outgoingWorld,
    &bsdf,
    &bsdfPdf
  );

  if (bsdfPdf > 0.0f && !math::isVectorExactlyZero(bsdf)) {
    // Evaluate light PDF as well.
    Vec lightColor;
    float lightPdf;
    evalLight(
      emissionObj,
      kdt,
      isect.position,
      outgoingWorld,
      &lightColor,
      &lightPdf
    );

    // There's a risk that the light PDF is zero (e.g. occluded light).
    if (lightPdf > 0.0f && !math::isVectorExactlyZero(lightColor)) {
      float bsdfWeight = math::powerHeuristic(1, bsdfPdf, 1, lightPdf);
      return bsdf.cwiseProduct(lightColor)
        * fabsf(isect.normal.dot(outgoingWorld))
        * bsdfWeight / bsdfPdf;
    }
  }

  return Vec(0, 0, 0);
}

inline Vec AreaLight::emit(
  const Vec& dirToLight,
  const Vec& normalOnLight
) const {
  // Only emit on the normal-facing side of objects, e.g. on the outside of a
  // sphere or on the normal side of a disc.
  if (dirToLight.dot(normalOnLight) < 0.0f) {
    // Direction to light faces the normal side.
    return color;
  } else {
    return Vec(0, 0, 0);
  }
}

void AreaLight::evalLight(
  const Geom* emissionObj,
  const KDTree& kdt,
  const Vec& point,
  const Vec& dirToLight,
  Vec* colorOut,
  float* pdfOut
) const {
  Ray pointToLight(point + math::VERY_SMALL * dirToLight, dirToLight);
  Intersection lightIsect;

  if (kdt.intersect(pointToLight, &lightIsect) != emissionObj) {
    // Something's blocking the light.
    *colorOut = Vec(0, 0, 0);
    *pdfOut = 0.0f;
    return;
  } else {
    const float dirToLightDist2 = (lightIsect.position - point).squaredNorm();
    const float absCosTheta = fabsf(lightIsect.normal.dot(-dirToLight));
    *colorOut = emit(dirToLight, lightIsect.normal);
    *pdfOut = dirToLightDist2 / (absCosTheta * emissionObj->area());
    return;
  }
}

void AreaLight::sampleLight(
  Randomness& rng,
  const Geom* emissionObj,
  const KDTree& kdt,
  const Vec& point,
  Vec* dirToLightOut,
  Vec* colorOut,
  float* pdfOut
) const {
  Vec pointOnLight;
  Vec normalOnLight;
  emissionObj->samplePoint(rng, &pointOnLight, &normalOnLight);

  const Vec dirToLight = (pointOnLight - point).normalized();
  const float dirToLightDist2 = (pointOnLight - point).squaredNorm();

  // Why do we subtract 2 * VERY_SMALL?
  // 1. To balance out the added VERY_SMALL in the ray pointToLight, and
  // 2. Because we don't want to actually hit the light itself!
  float shadowDist = sqrtf(dirToLightDist2) - 2.0f * math::VERY_SMALL;
  Ray pointToLight(point + math::VERY_SMALL * dirToLight, dirToLight);

  if (kdt.intersectShadow(pointToLight, shadowDist)) {
    // Something's blocking the light.
    *dirToLightOut = dirToLight;
    *colorOut = Vec(0, 0, 0);
    *pdfOut = 0.0f;
  } else {
    const float absCosTheta = fabsf(normalOnLight.dot(-dirToLight));
    *dirToLightOut = dirToLight;
    *colorOut = emit(dirToLight, normalOnLight);
    *pdfOut = dirToLightDist2 / (absCosTheta * emissionObj->area());
  }
}

Vec AreaLight::directIlluminate(
  Randomness& rng,
  const Ray& incoming,
  const Intersection& isect,
  const Material* mat,
  const Geom* emissionObj,
  const KDTree& kdt
) const {
  Vec Ld(0, 0, 0);
  Ld += directIlluminateByLightPDF(rng, incoming, isect, mat, emissionObj, kdt);
  Ld += directIlluminateByMatPDF(rng, incoming, isect, mat, emissionObj, kdt);

  return Ld;
}
