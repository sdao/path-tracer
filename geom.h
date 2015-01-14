#pragma once
#include "core.h"
#include "node.h"
#include "embree.h"

class Material;
class AreaLight;

/**
 * The base interface for all renderable geometry.
 */
class Geom {
  static void embreeBoundsFunc(void* user, size_t i, RTCBounds& bounds);
  static void embreeIntersectFunc(void* user, RTCRay& ray, size_t i);
  static void embreeOccludedFunc(void* user , RTCRay& ray, size_t i);
  static void embreeIntersectCallback(
    const Embree::EmbreeObj* eo,
    const RTCRay& ray,
    Intersection* isectOut
  );

protected:
  /**
   * Constructs a geom with the specified material.
   *
   * @param m the material used to render the geometry
   * @param l the area light causing emission from the geometry
   */
  Geom(const Material* m = nullptr, const AreaLight* l = nullptr);

  /**
   * Constructs a geom from the given node.
   */
  Geom(const Node& n);

public:
  const Material* mat; /**< The material used to render the geom. */
  const AreaLight* light; /**< The area light causing emission from the geom. */

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
   * Finds an intersection between the geometry and the given shadow ray.
   *
   * @param r       the shadow ray to find an intersection with
   * @param maxDist the maximum distance from the ray origin to the intersection
   * @returns       true if the ray hit the geometry within maxDist, false
   *                otherwise
   */
  virtual bool intersectShadow(const Ray& r, float maxDist) const = 0;

  /**
   * A bounding box encapsulating the entire geometry.
   */
  virtual BBox bounds() const = 0;

  /**
   * A bounding sphere encapsulating the entire geometry.
   * If this method is not overriden, then the bounding sphere is
   * automatically calculated from the bounding box.
   */
  virtual BSphere boundSphere() const;

  /**
   * Refines a composite object into its constituent parts until the
   * parts can be intersected.
   */
  virtual void refine(std::vector<const Geom*>& refined) const;

  /**
   * Makes an Embree geometry object from this geometry.
   * Composite objects may choose to create one Embree object composed of
   * multiple primitives if they can do so.
   *
   * @param scene the scene in which to create the Embree object
   * @param eo    the Embree object to populate
   */
  virtual void makeEmbreeObject(RTCScene scene, Embree::EmbreeObj& eo) const;
};
