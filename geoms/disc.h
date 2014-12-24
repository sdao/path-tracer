#pragma once
#include "../geom.h"

namespace geoms {

  /**
   * A geometric disc, that is, a plane area bounded by a circle.
   */
  class Disc : public Geom {
    const float radiusSquared; /** The pre-computed square of the radius. **/

  public:
    const float radius; /**< The distance from the disc's center to its edge. */
    const Vec origin; /**< The center of the disc. */
    const Vec normal; /**< A vector perpendicular to the disc's plane. */

    /**
     * Constructs a disc.
     *
     * @param m the material used to render the disc
     * @param o the center (origin) of the disc
     * @param n the normal vector perpendicular to the disc's plane
     * @param r the radius of the disc
     */
    Disc(
      Material* m,
      Vec o = Vec(0, 0, 0),
      Vec n = Vec(0, 1, 0),
      float r = 10.0f
    );

    /**
     * Constructs a disc from another disc.
     */
    Disc(const geoms::Disc& other);

    virtual bool intersect(const Ray& r, Intersection* isectOut) const override;
    virtual BBox bounds() const override;
  };

}
