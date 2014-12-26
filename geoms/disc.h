#pragma once
#include "../geom.h"

namespace geoms {

  /**
   * A geometric disc, that is, a plane area bounded by a circle.
   * Optionally, the disc can have a circular hole in the middle.
   */
  class Disc : public Geom {
    const float radiusOuterSquared; /** The square of the outer radius. **/
    const float radiusInnerSquared; /** The square of the inner radius. **/

  public:
    const float radiusOuter; /**< The center-to-outer-edge distance. */
    const float radiusInner; /**< The center-to-inner-edge distance. */
    const Vec origin; /**< The center of the disc. */
    const Vec normal; /**< A vector perpendicular to the disc's plane. */

    /**
     * Constructs a disc.
     *
     * @param m      the material used to render the disc
     * @param o      the center (origin) of the disc
     * @param n      the normal vector perpendicular to the disc's plane
     * @param rOuter the outer radius of the disc
     * @param rInner the inner radius of the disc (the radius of its hole)
     */
    Disc(
      Material* m,
      Vec o = Vec(0, 0, 0),
      Vec n = Vec(0, 1, 0),
      float rOuter = 10.0f,
      float rInner = 0.0f
    );

    /**
     * Constructs a disc from another disc.
     */
    Disc(const geoms::Disc& other);

    virtual bool intersect(const Ray& r, Intersection* isectOut) const override;
    virtual BBox bounds() const override;
  };

}
