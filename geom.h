#pragma once
#include "core.h"
#include "material.h"

/**
 * The base interface for all renderable geometry.
 */
class geom {
protected:
  /**
   * Constructs a geom with the specified material. All classes inheriting geom
   * must call this constructor in their own constructors.
   *
   * @param m the material used to render the geometry
   */
  geom(material* m);

public:
  material* mat; /**< The material used to render the geometry. */

  virtual ~geom();
  
  /**
   * Finds an intersection between the geometry and the given ray.
   *
   * @param r the ray to find an intersection with
   * @returns the intersection information; to see whether the ray intersected
   *          the geometry, use intersection::hit()
   */
  virtual intersection intersect(const ray& r) const = 0;
  
  /**
   * A bounding box encapsulating the entire geometry.
   */
  virtual bbox bounds() const = 0;
};
