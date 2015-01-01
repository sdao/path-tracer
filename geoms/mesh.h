#pragma once
#include "../geom.h"
#include <vector>
#include "poly.h"

namespace geoms {

  /**
   * A collection of polys loaded from an external 3D model file.
   */
  class Mesh : public Geom {
    std::vector<Poly::Point> points; /**< The point lookup table. */
    std::vector<Poly> faces; /**< The faces of the mesh. */

  public:
    const Vec origin;

    /**
     * Constructs an empty mesh. Use mesh::readPolyModel() to populate.
     *
     * @param o the origin of the mesh in world space
     * @param m the material used to render the mesh
     * @param l the area light causing emission from the mesh
     */
    Mesh(Vec o, Material* m = nullptr, AreaLight* l = nullptr);

    /**
     * Reads a polygon model from a file and populates a mesh.
     * Note: any previous data will be destroyed. All existing pointers to this
     * data will be invalidated.
     *
     * @param name the name of the file to read; can be any file format that the
     *             Open Asset Import Library recognizes (e.g. obj)
     * @returns    true if the mesh could be read, false otherwise
     */
    bool readPolyModel(std::string name);

    /**
     * Destroys the existing poly faces and points in the mesh.
     * All existing pointers to this data will be invalidated.
     */
    void clear();

    virtual bool intersect(const Ray& r, Intersection* isectOut) const override;
    virtual bool intersectShadow(const Ray& r, float maxDist) const override;
    virtual BBox bounds() const override;
    virtual Vec samplePoint(Randomness& rng) const override;
    virtual float area() const override;
    virtual void refine(std::vector<const Geom*>& refined) const override;
  };

}
