#pragma once
#include "../geom.h"

namespace geoms {

  /**
   * A geometric disc, that is, a plane area bounded by a circle.
   */
  class disc : public geom {
    vec tangent; /**< A vector in the plane of the disc. **/
    vec binormal; /**< A second orthogonal vector in the plane of the disc. **/
    const float radiusSquared; /** The pre-computed square of the radius. **/

  public:
    const vec origin; /**< The center of the disc. */
    const vec normal; /**< A vector perpendicular to the disc's plane. */
    const float radius; /**< The distance from the disc's center to its edge. */

    /**
     * Constructs a disc.
     *
     * @param m the material used to render the disc
     * @param o the center (origin) of the disc
     * @param n the normal vector perpendicular to the disc's plane
     * @param r the radius of the disc
     */
    disc(
      material* m,
      vec o = vec(0, 0, 0),
      vec n = vec(0, 1, 0),
      float r = 10.0f
    );
    
    /**
     * Constructs a disc from another disc.
     */
    disc(const geoms::disc& other);

    virtual bool intersect(const ray& r, intersection* isectOut) const;
    virtual bbox bounds() const;
  };

}
