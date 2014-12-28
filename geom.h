#pragma once
#include "core.h"

class Material;
class AreaLight;

/**
 * The base interface for all renderable geometry.
 */
class Geom {
protected:
  /**
   * Constructs a geom with the specified material.
   *
   * @param m the material used to render the geometry
   * @param l the area light causing emission from the geometry
   */
  Geom(Material* m = nullptr, AreaLight* l = nullptr);

public:
  Material* mat; /**< The material used to render the geometry. */
  AreaLight* light; /**< The area light causing emission from the geometry. */

  virtual ~Geom();

  /**
   * Finds an intersection between the geometry and the given ray.
   *
   * @param r              the ray to find an intersection with
   * @param isectOut [out] the intersection information if the ray hit the
   *                       geometry, otherwise unmodified; the pointer must not
   *                       be null
   * @returns              true if the ray hit the geometry, false otherwise
   */
  virtual bool intersect(const Ray& r, Intersection* isectOut) const = 0;

  /**
   * A bounding box encapsulating the entire geometry.
   */
  virtual BBox bounds() const = 0;

  /**
   * Randomly samples a point on the geometry, uniform with respect to the area
   * of the geometry.
   */
  virtual Vec samplePoint(Randomness& rng) const = 0;

  /**
   * Gets the total surface area of the geometry.
   */
  virtual float area() const = 0;
};
