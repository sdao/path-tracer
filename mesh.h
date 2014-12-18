#pragma once
#include "geoms/poly.h"
#include <vector>

class mesh {
    std::vector<geoms::poly::point> points;
    std::vector<geoms::poly> faces;
  
public:
    mesh();

    /**
     * Reads a polygon model from a file and creates a mesh.
     * As a convenience, if a geometry vector is specified as the last
     * parameter, pointers to all the poly faces in the mesh will be appended
     * to the end. (Alternatively, call mesh::appendFacesTo(...) later.)
     *
     * Returns a pointer to the generated mesh. The caller of this function
     * will own the pointer.
     *
     * Note: the previous data will be destroyed. See mesh::clear(...).
     */
    bool readPolyModel(
      material* m,
      std::string name,
      vec offset,
      std::vector<geom*>* geomList = nullptr
    );
  
    /**
     * Appends a pointer to each poly face of the mesh to the specified
     * geometry vector.
     */
    void appendFacesTo(std::vector<geom*>* geomList);
  
    /**
     * Destroys the existing poly faces and points in the mesh.
     * All existing pointers to this data will be invalidated.
     */
    void clear();
};
