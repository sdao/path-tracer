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
  float outgoingDist;
  Vec lightColor;
  float lightPdf;
  sampleLight(
    rng,
    emissionObj,
    isect.position,
    &outgoingWorld,
    &outgoingDist,
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

    if (bsdfPdf > 0.0f && !math::isVectorExactlyZero(bsdf)) {
      // There's a risk that something's blocking between the object and light.
      bool occluded = kdt.intersectShadow(
        Ray(isect.position + math::VERY_SMALL * outgoingWorld, outgoingWorld),
        // Why do we subtract 2 * VERY_SMALL?
        // 1. To balance out the added VERY_SMALL in the ray pointToLight, and
        // 2. Because we don't want to actually hit the light itself!
        outgoingDist - 2.0f * math::VERY_SMALL
      );

      if (!occluded) {
        float lightWeight = math::powerHeuristic(1, lightPdf, 1, bsdfPdf);
        return bsdf.cwiseProduct(lightColor)
          * fabsf(isect.normal.dot(outgoingWorld))
          * lightWeight / lightPdf;
      }
    }
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
    float outgoingDist;
    float lightPdf;
    evalLight(
      emissionObj,
      isect.position,
      outgoingWorld,
      &outgoingDist,
      &lightColor,
      &lightPdf
    );

    if (lightPdf > 0.0f && !math::isVectorExactlyZero(lightColor)) {
      // There's a risk that something's blocking between the object and light.
      bool occluded = kdt.intersectShadow(
        Ray(isect.position + math::VERY_SMALL * outgoingWorld, outgoingWorld),
        // Why do we subtract 2 * VERY_SMALL?
        // 1. To balance out the added VERY_SMALL in the ray pointToLight, and
        // 2. Because we don't want to actually hit the light itself!
        outgoingDist - 2.0f * math::VERY_SMALL
      );

      if (!occluded) {
        float bsdfWeight = math::powerHeuristic(1, bsdfPdf, 1, lightPdf);
        return bsdf.cwiseProduct(lightColor)
          * fabsf(isect.normal.dot(outgoingWorld))
          * bsdfWeight / bsdfPdf;
      }
    }
  }

  return Vec(0, 0, 0);
}

inline Vec AreaLight::emit(
  const Intersection& isect,
  const Vec& direction
) const {
  // Only emit on the normal-facing side of objects, e.g. on the outside of a
  // sphere or on the normal side of a disc.
  if (direction.dot(isect.normal) < 0.0f) {
    // Incoming direction towards light is opposite (facing) the normal.
    return color;
  } else {
    return Vec(0, 0, 0);
  }
}

void AreaLight::evalLight(
  const Geom* emissionObj,
  const Vec& point,
  const Vec& dirToLight,
  float* distToLightOut,
  Vec* colorOut,
  float* pdfOut
) const {
  Ray pointToLight(point + math::VERY_SMALL * dirToLight, dirToLight);
  Intersection lightIsect;

  if (!emissionObj->intersect(pointToLight, &lightIsect)) {
    // The ray doesn't even hit the light (ignoring other scene objects).
    // So the light wouldn't have ever chosen this direction at all when
    // sampling random directions.

    *distToLightOut = 0.0f;
    *colorOut = Vec(0, 0, 0);
    *pdfOut = 0.0f;
  } else {
    const float dirToLightDist2 = (lightIsect.position - point).squaredNorm();
    const float absCosTheta = fabsf(lightIsect.normal.dot(-dirToLight));

    *distToLightOut = sqrtf(dirToLightDist2);
    *colorOut = emit(lightIsect, dirToLight);
    *pdfOut = dirToLightDist2 / (absCosTheta * emissionObj->area());
  }
}

void AreaLight::sampleLight(
  Randomness& rng,
  const Geom* emissionObj,
  const Vec& point,
  Vec* dirToLightOut,
  float* distToLightOut,
  Vec* colorOut,
  float* pdfOut
) const {
  // Note: this is not the final point; we have really only sampled the
  // direction. (Consider a sphere where we sample a point on the opposite side
  // of the observer being illuminated. The direction sampled will actually
  // cause an intersection with some other point on the side facing the
  // observer.)
  const Vec sample = emissionObj->samplePoint(rng);
  const Vec dirToLight = (sample - point).normalized();

  *dirToLightOut = dirToLight;
  evalLight(emissionObj, point, dirToLight, distToLightOut, colorOut, pdfOut);
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
