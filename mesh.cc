#include "mesh.h"
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

Mesh::Mesh() : points(), faces() {}

bool Mesh::readPolyModel(
  std::string name,
  Vec offset,
  Material* m,
  std::vector<Geom*>* geomList
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
  if (!scene) {
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

    // We have something to work with! Clear the existing data.
    clear();

    // Add points.
    for (size_t i = 0; i < mesh->mNumVertices; ++i) {
      aiVector3D thisPos = mesh->mVertices[i];
      aiVector3D thisNorm = mesh->mNormals[i];

      geoms::Poly::Point thisPoint;
      thisPoint.position = Vec(thisPos.x, thisPos.y, thisPos.z) + offset;
      thisPoint.normal =
        Vec(thisNorm.x, thisNorm.y, thisNorm.z).normalized();

      points.push_back(thisPoint);
    }

    // Add faces.
    for (size_t i = 0; i < mesh->mNumFaces; ++i) {
      aiFace face = mesh->mFaces[i];

      // Only add the triangles (we should have a triangulated mesh).
      if (face.mNumIndices == 3) {
        geoms::Poly thisPoly(
          mem::ID(face.mIndices[0]),
          mem::ID(face.mIndices[1]),
          mem::ID(face.mIndices[2]),
          &points,
          m
        );

        faces.push_back(thisPoly);
      }
    }
  }

  if (geomList) {
    appendFacesTo(geomList);
  }

  // We're done. Everything will be cleaned up by the importer destructor.
  return true;
}

void Mesh::appendFacesTo(std::vector<Geom*>* geomList) {
  if (geomList) {
    for (size_t i = 0; i < faces.size(); ++i) {
      geomList->push_back(&faces[i]);
    }
  }
}

void Mesh::clear() {
  points.clear();
  faces.clear();
}
