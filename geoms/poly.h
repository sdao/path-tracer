#pragma once
#include "../geom.h"
#include "../mem.h"

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

    const mem::ID pt0; /** < The index of the first point (in CCW order). */
    const mem::ID pt1; /** < The index of the second point (in CCW order). */
    const mem::ID pt2; /** < The index of the third point (in CCW order). */
    /** A point lookup table. We DO NOT own the pointer. */
    std::vector<geoms::Poly::Point>* pointLookup;

    /**
     * Constructs a poly.
     *
     * @param a          the index of the first point (in CCW winding order)
     * @param b          the index of the second point (in CCW winding order)
     * @param c          the index of the third point (in CCW winding order)
     * @param lookup     a pointer to a point lookup table
     * @param m          the material used to render the poly
     * @param l          the area light causing emission from the poly
     */
    Poly(
      mem::ID a,
      mem::ID b,
      mem::ID c,
      std::vector<geoms::Poly::Point>* lookup,
      const Material* m = nullptr,
      const AreaLight* l = nullptr
    );
    /**
     * Constructs a poly from another poly.
     */
    Poly(const geoms::Poly& other);

    virtual bool intersect(const Ray& r, Intersection* isectOut) const override;
    virtual bool intersectShadow(const Ray& r, float maxDist) const override;
    virtual BBox bounds() const override;
    virtual void samplePoint(
      Randomness& rng,
      Vec* positionOut,
      Vec* normalOut
    ) const override;
    virtual float area() const override;

  private:
    /**
     * Gets the actual point data for the given point index by consulting
     * the point lookup table.
     *
     * @param i the index to look up in the table
     * @return  the point at the given index
     */
    inline const Point get(mem::ID i) const {
      return mem::ref(*pointLookup, i);
    }
  };

}
