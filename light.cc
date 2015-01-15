#include "light.h"
#include "accelerator.h"

AreaLight::AreaLight(const Vec& c) : color(c) {}

AreaLight::AreaLight(const Node& n) : AreaLight(n.getVec("color")) {}

inline Vec AreaLight::directIlluminateByLightPDF(
  Randomness& rng,
  const Ray& incoming,
  const Intersection& isect,
  const Material* mat,
  const Geom* emissionObj,
  const Accelerator* accel
) const {
  // Sample random from light PDF.
  Vec outgoingWorld;
  Vec lightColor;
  float lightPdf;
  sampleLight(
    rng,
    accel,
    emissionObj,
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

    if (bsdfPdf > 0.0f && !math::isVectorExactlyZero(bsdf)) {
      float lightWeight = math::powerHeuristic(1, lightPdf, 1, bsdfPdf);
      return bsdf.cwiseProduct(lightColor)
        * fabsf(isect.normal.dot(outgoingWorld))
        * lightWeight / lightPdf;
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
  const Accelerator* accel
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
      accel,
      emissionObj,
      isect.position,
      outgoingWorld,
      &lightColor,
      &lightPdf
    );

    if (lightPdf > 0.0f && !math::isVectorExactlyZero(lightColor)) {
      float bsdfWeight = math::powerHeuristic(1, bsdfPdf, 1, lightPdf);
      return bsdf.cwiseProduct(lightColor)
        * fabsf(isect.normal.dot(outgoingWorld))
        * bsdfWeight / bsdfPdf;
    }
  }

  return Vec(0, 0, 0);
}

Vec AreaLight::emit(
  const Ray& incoming,
  const Intersection& isect
) const {
  // Only emit on the normal-facing side of objects, e.g. on the outside of a
  // sphere or on the normal side of a disc.
  if (incoming.direction.dot(isect.normal) > 0.0f) {
    return Vec(0, 0, 0);
  }

  return color;
}

Vec AreaLight::emit(
  const Ray& incoming,
  const Intersection& isect,
  const Accelerator* accel
) const {
  // Only emit on the normal-facing side of objects, e.g. on the outside of a
  // sphere or on the normal side of a disc.
  if (incoming.direction.dot(isect.normal) > 0.0f) {
    return Vec(0, 0, 0);
  }

  // Object might be occluded behind another object.
  float dist = isect.distance - 2.0f * math::VERY_SMALL;
  if (accel->intersectShadow(incoming, dist)) {
    return Vec(0, 0, 0);
  }

  return color;
}

void AreaLight::evalLight(
  const Accelerator* accel,
  const Geom* emissionObj,
  const Vec& point,
  const Vec& dirToLight,
  Vec* colorOut,
  float* pdfOut
) const {
  float pdf;
  Vec emittedColor;

  BSphere emitterBounds = emissionObj->boundSphere();
  if (emitterBounds.contains(point)) {
    // We're inside the bounding sphere, so sample sphere uniformly.
    pdf = math::uniformSampleSpherePDF();
  } else {
    // We're outside the bounding sphere, so sample by solid angle.
    Vec dirToLightOrigin = emitterBounds.origin - point;
    float theta = asinf(emitterBounds.radius / dirToLightOrigin.norm());

    Vec normal = dirToLightOrigin.normalized();
    Vec tangent;
    Vec binormal;
    math::coordSystem(normal, &tangent, &binormal);

    Vec dirToLightLocal =
      math::worldToLocal(dirToLight, tangent, binormal, normal);
    pdf = math::uniformSampleConePDF(theta, dirToLightLocal);
  }

  Ray pointToLight(point + math::VERY_SMALL * dirToLight, dirToLight);
  Intersection lightIsect;
  if (!emissionObj->intersect(pointToLight, &lightIsect)) {
    // No emission if the ray doesn't hit the light
    // (e.g. sampling doesn't exactly correspond with light).
    emittedColor = Vec(0, 0, 0);
  } else {
    // Emits color if the ray does hit the light.
    emittedColor = emit(pointToLight, lightIsect, accel);
  }

  *colorOut = emittedColor;
  *pdfOut = pdf;
}

void AreaLight::sampleLight(
  Randomness& rng,
  const Accelerator* accel,
  const Geom* emissionObj,
  const Vec& point,
  Vec* dirToLightOut,
  Vec* colorOut,
  float* pdfOut
) const {
  Vec dirToLight;
  float pdf;
  Vec emittedColor;

  BSphere emitterBounds = emissionObj->boundSphere();
  if (emitterBounds.contains(point)) {
    // We're inside the bounding sphere, so sample sphere uniformly.
    dirToLight = math::uniformSampleSphere(rng);
    pdf = math::uniformSampleSpherePDF();
  } else {
    // We're outside the bounding sphere, so sample by solid angle.
    Vec dirToLightOrigin = emitterBounds.origin - point;
    float theta = asinf(emitterBounds.radius / dirToLightOrigin.norm());

    Vec normal = dirToLightOrigin.normalized();
    Vec tangent;
    Vec binormal;
    math::coordSystem(normal, &tangent, &binormal);

    dirToLight = math::localToWorld(
      math::uniformSampleCone(rng, theta),
      tangent,
      binormal,
      normal
    );
    pdf = math::uniformSampleConePDF(theta);
  }

  Ray pointToLight(point + math::VERY_SMALL * dirToLight, dirToLight);
  Intersection lightIsect;
  if (!emissionObj->intersect(pointToLight, &lightIsect)) {
    // No emission if the ray doesn't hit the light
    // (e.g. sampling doesn't exactly correspond with light).
    emittedColor = Vec(0, 0, 0);
  } else {
    // Emits color if the ray does hit the light.
    emittedColor = emit(pointToLight, lightIsect, accel);
  }

  *dirToLightOut = dirToLight;
  *colorOut = emittedColor;
  *pdfOut = pdf;
}

Vec AreaLight::directIlluminate(
  Randomness& rng,
  const Ray& incoming,
  const Intersection& isect,
  const Material* mat,
  const Geom* emissionObj,
  const Accelerator* accel
) const {
  Vec Ld(0, 0, 0);
  
  Ld +=
    directIlluminateByLightPDF(rng, incoming, isect, mat, emissionObj, accel);
  Ld +=
    directIlluminateByMatPDF(rng, incoming, isect, mat, emissionObj, accel);

  return Ld;
}
