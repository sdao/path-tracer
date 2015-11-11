#pragma once
#include "../geom.h"

namespace geoms {

  /**
   * A geometric polygon with three points, i.e. a triangle.
   */
  class Poly : public Geom {
  public:
    /**
     * A point of a poly. Can be shared between multiple polys.
     */
    struct Point {
      Vec position; /**< The position of the point in 3D space. */
      Vec normal; /**< The normal of the surface at the point. */
    };

    const Point* pt0; /**< Pointer to the first point (in CCW order). */
    const Point* pt1; /**< Pointer to the second point (in CCW order). */
    const Point* pt2; /**< Pointer to the third point (in CCW order). */

    /**
     * Constructs a poly.
     *
     * @param a          a pointer to the first point (in CCW winding order)
     * @param b          a pointer to the second point (in CCW winding order)
     * @param c          a pointer to the third point (in CCW winding order)
     * @param m          the material used to render the poly
     * @param l          the area light causing emission from the poly
     */
    Poly(
      const Point* a,
      const Point* b,
      const Point* c,
      const Material* m = nullptr,
      const AreaLight* l = nullptr
    );

    /**
     * Constructs a poly from another poly.
     */
    Poly(const geoms::Poly& other);

    virtual bool intersect(const Ray& r, Intersection* isectOut) const override;
    virtual bool intersectShadow(const Ray& r, float maxDist) const override;
    virtual BBox boundBox() const override;
    virtual void sampleRay(
      Randomness& rng,
      Ray* rayOut,
      float* pdfPosOut,
      float* pdfDirOut
    ) const override;
    virtual float area() const override;
  };

}
