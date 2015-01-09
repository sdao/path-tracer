#pragma once
#include "../geom.h"
#include <vector>
#include <exception>
#include "poly.h"

namespace geoms {

  /**
   * A collection of polys loaded from an external 3D model file.
   */
  class Mesh : public Geom {
    std::vector<Poly::Point> points; /**< The point lookup table. */
    std::vector<Poly> faces; /**< The faces of the mesh. */

  private:
    /**
     * Reads a polygon model from a file and populates a mesh.
     *
     * @param name the name of the file to read; can be any file format that the
     *             Open Asset Import Library recognizes (e.g. obj)
     *
     * @throws geoms::Mesh::MeshFileImportError if the file couldn't be read
     */
    void readPolyModel(std::string name);

  public:
    const Vec origin;

    /**
     * Constructs a mesh from a polygon model file on disk.
     *
     * @param o    the origin of the mesh in world space
     * @param name the name of the file to read; can be any file format that the
     *             Open Asset Import Library recognizes (e.g. obj)
     * @param m    the material used to render the mesh
     * @param l    the area light causing emission from the mesh
     *
     * @throws geoms::Mesh::MeshFileImportError if the file couldn't be read
     */
    Mesh(
      Vec o,
      std::string name,
      const Material* m = nullptr,
      const AreaLight* l = nullptr
    );

    virtual bool intersect(const Ray& r, Intersection* isectOut) const override;
    virtual bool intersectShadow(const Ray& r, float maxDist) const override;
    virtual BBox bounds() const override;
    virtual Vec samplePoint(Randomness& rng) const override;
    virtual float area() const override;
    virtual void refine(std::vector<const Geom*>& refined) const override;
  };

}
