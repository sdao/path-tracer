#include "poly.h"
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags
#include <iostream>

geoms::poly::poly(
  material* m,
  mem::id a,
  mem::id b,
  mem::id c,
  std::vector<geoms::poly::point>* lookup
) : geom(m), pt0(a), pt1(b), pt2(c), pointLookup(lookup) {}

geoms::poly::poly(const geoms::poly& other)
  : geom(other.mat), pt0(other.pt0), pt1(other.pt1), pt2(other.pt2),
    pointLookup(other.pointLookup) {}

intersection geoms::poly::intersect(const ray& r) const {
  // Uses the Moller-Trumbore intersection algorithm.
  // See <http://en.wikipedia.org/wiki/
  // M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm> for more info.
  
  const point& pt0_data = get(pt0);
  const point& pt1_data = get(pt1);
  const point& pt2_data = get(pt2);
  
  const vec edge1 = pt1_data.position - pt0_data.position;
  const vec edge2 = pt2_data.position - pt0_data.position;
  
  const vec p = glm::cross(r.direction, edge2);
  const float det = glm::dot(edge1, p);
  
  if (math::isNearlyZero(det)) {
    return intersection(); // No hit on plane.
  }
  
  const float invDet = 1.0f / det;
  const vec t = r.origin - pt0_data.position;
  
  const float u = glm::dot(t, p) * invDet;
  if (u < 0.0f || u > 1.0f) {
    return intersection(); // In plane but not triangle.
  }
  
  const vec q = glm::cross(t, edge1);
  const float v = glm::dot(r.direction, q) * invDet;
  if (v < 0.0f || (u + v) > 1.0f) {
    return intersection(); // In plane but not triangle.
  }
  
  const float dist = glm::dot(edge2, q) * invDet;
  if (dist < 0.0f) {
    return intersection(); // In triangle but behind us.
  }
  
  const float w = 1.0f - u - v;
  
  return intersection(
    r.at(dist),
    w * pt0_data.normal + u * pt1_data.normal + v * pt2_data.normal,
    w * pt0_data.tangent + u * pt1_data.tangent + v * pt2_data.tangent,
    w * pt0_data.binormal + u * pt1_data.binormal + v * pt2_data.binormal,
    dist
  );
}

bbox geoms::poly::bounds() const {
  bbox b(get(pt0).position, get(pt1).position);
  b.expand(get(pt2).position);
  
  return b;
}

bool geoms::poly::readPolyModel(
  material* m,
  std::string name,
  std::vector<geoms::poly::point>& pointLookup,
  std::vector<geoms::poly>& polys,
  vec offset
) {
  // Create an instance of the Importer class
  Assimp::Importer importer;
  
  // And have it read the given file with some example postprocessing.
  const aiScene* scene = importer.ReadFile(
    name,
    aiProcess_Triangulate
    | aiProcess_JoinIdenticalVertices
    | aiProcess_SortByPType
    | aiProcess_GenNormals
    | aiProcess_PreTransformVertices
    | aiProcess_ValidateDataStructure
  );
  
  // If the import failed, report it
  if(!scene)
  {
    std::cerr << "Importer error: " << importer.GetErrorString() << "\n";
    return false;
  }
  
  // Now we can access the file's contents.
  if (scene->mNumMeshes > 0) {
    // Process first mesh only right now.
    // TODO: process multiple meshes.
    aiMesh* mesh = scene->mMeshes[0];
    
    if (!mesh->HasPositions()) {
      std::cerr << "Error: no vertex positions on the mesh.\n";
      return false;
    }
    
    if (!mesh->HasNormals()) {
      std::cerr << "Error: no vertex normals on the mesh.\n";
      return false;
    }
    
    // Add points.
    mem::id pointOffset = pointLookup.size();
    for (size_t i = 0; i < mesh->mNumVertices; ++i) {
      aiVector3D thisPos = mesh->mVertices[i];
      aiVector3D thisNorm = mesh->mNormals[i];
      
      point thisPoint;
      thisPoint.position = vec(thisPos.x, thisPos.y, thisPos.z) + offset;
      thisPoint.normal = glm::normalize(
        vec(thisNorm.x, thisNorm.y, thisNorm.z)
      );
      thisPoint.computeTangents();
      
      pointLookup.push_back(thisPoint);
    }
    
    // Add faces.
    for (size_t i = 0; i < mesh->mNumFaces; ++i) {
      aiFace face = mesh->mFaces[i];
      
      // Only add the triangles (we should have a triangulated mesh).
      if (face.mNumIndices == 3) {
        poly thisPoly(
          m,
          face.mIndices[0] + pointOffset,
          face.mIndices[1] + pointOffset,
          face.mIndices[2] + pointOffset,
          &pointLookup
        );
        
        polys.push_back(thisPoly);
      }
    }
  }
  
  // We're done. Everything will be cleaned up by the importer destructor.
  return true;
}
