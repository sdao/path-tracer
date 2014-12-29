#pragma once
#include "core.h"
#include "geom.h"
#include "material.h"
#include "kdtree.h"

/**
 * A diffuse area light that causes radiance to be emitted from a piece of
 * solid geometry.
 */
class AreaLight {
private:
  /**
   * Helper function for AreaLight::directIlluminate.
   * Calculates only the weighted component of direct illumination according
   * to the light's PDF.
   * For help on parameters, see the documentation for
   * AreaLight::directIlluminate.
   */
  inline Vec directIlluminateByLightPDF(
    Randomness& rng,
    const Ray& incoming,
    const Intersection& isect,
    const Material* mat,
    const Geom* emissionObj,
    const KDTree& kdt
  ) const;

  /**
   * Helper function for AreaLight::directIlluminate.
   * Calculates only the weighted component of direct illumination according
   * to the material's PDF.
   * For help on parameters, see the documentation for
   * AreaLight::directIlluminate.
   */
  inline Vec directIlluminateByMatPDF(
    Randomness& rng,
    const Ray& incoming,
    const Intersection& isect,
    const Material* mat,
    const Geom* emissionObj,
    const KDTree& kdt
  ) const;

  /**
   * Calculates the emittance of the area light via a given intersection on
   * the light, in a given direction towards the light.
   *
   * @param isect     the intersection on the light
   * @param direction the incoming direction from the ray-caster towards the
   *                  light
   * @returns         the emittance from the light
   */
  inline Vec emit(
    const Intersection& isect,
    const Vec& direction
  ) const;

public:
  const Vec color; /**< The color of the light emitted. */

  /**
   * Constructs a light that emits the specified color.
   */
  AreaLight(Vec c);

  /**
   * Evaluates the emittance from an emission object onto a given point via
   * a specified direction. (Note that a diffuse area light can illuminate a
   * point from multiple different directions.)
   *
   * @param emissionObj          the geometry from which light is emitted
   * @param point                the world-space point being illuminated by the
   *                             emissionObj
   * @param dirToLight           the direction from the world-space point
   *                             towards the light for which illumination
   *                             should be calculated
   * @param distToLightOut [out] the distance from the world-space point towards
   *                             the emissionObj via the direction dirToLight
   * @param colorOut       [out] the color the light emits
   * @param pdfOut         [out] the probability that the light-sampling routine
   *                             Material::sampleLight would have chosen the
   *                             direction dirToLight to illuminate the
   *                             world-space point
   */
  void evalLight(
    const Geom* emissionObj,
    const Vec& point,
    const Vec& dirToLight,
    float* distToLightOut,
    Vec* colorOut,
    float* pdfOut
  ) const;

  /**
   * Samples the emittance from the emission object onto a given point via a
   * randomly-chosen direction. (Note that a diffuse area light can illuminate a
   * point from multiple different directions.)
   *
   * @param rng                  the per-thread RNG in use
   * @param emissionObj          the geometry from which light is emitted
   * @param point                the world-space point being illuminated by the
   *                             emissionObj
   * @param dirToLightOut  [out] the randomly-sampled direction from the point
   *                             towards the emission object
   * @param distToLightOut [out] the distance from the world-space point towards
   *                             the emissionObj via the direction dirToLight
   * @param colorOut       [out] the color the light emits
   * @param pdfOut         [out] the probability of choosing the direction
   *                             dirToLight
   */
  void sampleLight(
    Randomness& rng,
    const Geom* emissionObj,
    const Vec& point,
    Vec* dirToLightOut,
    float* distToLightOut,
    Vec* colorOut,
    float* pdfOut
  ) const;

  /**
   * Computes the direct illumination from a random point on a piece of solid
   * geometry (the emitter) onto another piece of geometry (the reflector) at
   * the specified intersection point.
   *
   * @param rng             the per-thread RNG in use
   * @param incoming        the ray coming into the intersection on the target
   *                        geometry (on the reflector)
   * @param isect           the intersection on the target geometry that should
   *                        be illuminated
   * @param mat             the material of the target geometry being
   *                        illuminated
   * @param emissionObj     the object doing the illuminating (the emitter)
   * @param kdt             the k-d tree containing all geometry in the scene
   */
  Vec directIlluminate(
    Randomness& rng,
    const Ray& incoming,
    const Intersection& isect,
    const Material* mat,
    const Geom* emissionObj,
    const KDTree& kdt
  ) const;
};
