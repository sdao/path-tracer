#pragma once
#include "../geom.h"

namespace geoms {

  /**
   * A geometric sphere.
   */
  class sphere : public geom {
  public:
    const vec origin; /**< The center of the sphere. */
    const float radius; /**< The distance between the center and the surface. */

    /**
     * Constructs a sphere.
     *
     * @param m the material used to render the sphere
     * @param o the center (origin) of the sphere
     * @param r the radius of the sphere
     */
    sphere(material* m, vec o = vec(0, 0, 0), float r = 1.0f);
    
    /**
     * Constructs a sphere from another sphere.
     */
    sphere(const geoms::sphere& other);

    virtual intersection intersect(const ray& r) const;
    virtual bbox bounds() const;
  };

}
