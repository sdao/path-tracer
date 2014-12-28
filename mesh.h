#pragma once
#include "geoms/poly.h"
#include <vector>

/**
 * A collection of polys loaded from an external 3D model file.
 */
class Mesh {
    std::vector<geoms::Poly::Point> points; /**< The point lookup table. */
    std::vector<geoms::Poly> faces; /**< The faces of the mesh. */

public:
    /** Constructs an empty mesh. Use mesh::readPolyModel() to populate. */
    Mesh();

    /**
     * Reads a polygon model from a file and populates a mesh.
     * Note: any previous data will be destroyed. All existing pointers to this
     * data will be invalidated.
     *
     * @param m        the material used to render the poly faces of the mesh
     * @param name     the name of the file to read; can be any file format that
                       the Open Asset Import Library recognizes (e.g. obj)
     * @param offset   a vector offset to add to every point in the mesh
     * @param geomList as a convenience, if a geometry vector is specified,
     *                 pointers to all the poly faces in the mesh will be
     *                 appended to the end of geomList; alternatively, leave
     *                 null and use mesh::appendFacesTo(...) later
     *
     * @returns        true if the mesh could be read, false otherwise
     */
    bool readPolyModel(
      std::string name,
      Vec offset,
      Material* m,
      std::vector<Geom*>* geomList = nullptr
    );

    /**
     * Appends a pointer to each poly face of the mesh to the specified
     * geometry vector. (The given geometry vector must already exist.)
     */
    void appendFacesTo(std::vector<Geom*>* geomList);

    /**
     * Destroys the existing poly faces and points in the mesh.
     * All existing pointers to this data will be invalidated.
     */
    void clear();
};
