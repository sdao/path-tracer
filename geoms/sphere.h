#pragma once
#include "../geom.h"

namespace geoms {

  /**
   * A geometric sphere.
   */
  class Sphere : public Geom {
  public:
    const Vec origin; /**< The center of the sphere. */
    const float radius; /**< The distance between the center and the surface. */
    const bool inverted; /**< Whether the normals face inward. */

    /**
     * Constructs a sphere.
     *
     * @param o the center (origin) of the sphere
     * @param r the radius of the sphere
     * @param i whether the sphere's normals are inverted to face inward
     * @param m the material used to render the sphere
     * @param l the area light causing emission from the sphere
     */
    Sphere(
      const Vec& o = Vec(0, 0, 0),
      float r = 1.0f,
      bool i = false,
      const Material* m = nullptr,
      const AreaLight* l = nullptr
    );

    /**
     * Constructs a sphere from another sphere.
     */
    Sphere(const geoms::Sphere& other);

    /**
     * Constructs a sphere from the given node.
     */
    Sphere(const Node& n);

    virtual bool intersect(const Ray& r, Intersection* isectOut) const override;
    virtual bool intersectShadow(const Ray& r, float maxDist) const override;
    virtual BBox boundBox() const override;
    virtual BSphere boundSphere() const override;
  };

}
