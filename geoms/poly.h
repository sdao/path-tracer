#pragma once
#include "../geom.h"
#include "../mem.h"

namespace geoms {
  
  /**
   * A geometric polygon with three points, i.e. a triangle.
   */
  class poly : public geom {
  public:
    /**
     * A point of a poly. Can be shared between multiple polys.
     */
    struct point {
      vec position; /**< The position of the point in 3D space. */
      vec normal; /**< The normal of the surface at the point. */
      vec tangent; /**< A vector tangent to the surface at the point. */
      
      /**
       * Computes the tangent vector for the point from its normal.
       */
      inline void computeTangents() {
        vec dummy;
        math::coordSystem(normal, &tangent, &dummy);
      }
    };
    
    const mem::id pt0; /** < The index of the first point (in CCW order). */
    const mem::id pt1; /** < The index of the second point (in CCW order). */
    const mem::id pt2; /** < The index of the third point (in CCW order). */
    /** A point lookup table. We DO NOT own the pointer. */
    std::vector<geoms::poly::point>* pointLookup;
    
    /**
     * Constructs a poly.
     *
     * @param m          the material used to render the poly
     * @param a          the index of the first point (in CCW winding order)
     * @param b          the index of the second point (in CCW winding order)
     * @param c          the index of the third point (in CCW winding order)
     * @param lookup     a pointer to a point lookup table
     */
    poly(
      material* m,
      mem::id a,
      mem::id b,
      mem::id c,
      std::vector<geoms::poly::point>* lookup
    );
    /**
     * Constructs a poly from another poly.
     */
    poly(const geoms::poly& other);
    
    virtual bool intersect(const ray& r, intersection* isectOut) const;
    virtual bbox bounds() const;
    
  private:
    /**
     * Gets the actual point data for the given point index by consulting
     * the point lookup table.
     *
     * @param i the index to look up in the table
     * @return  the point at the given index
     */
    inline const point get(mem::id i) const {
      return mem::ref(*pointLookup, i);
    }
  };
  
}
