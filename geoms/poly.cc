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

intersection geoms::poly::intersect(const ray& /* r */) const {
  return intersection();
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
