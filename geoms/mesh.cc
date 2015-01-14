#include "mesh.h"
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags
#include <exception>
#include "../debug.h"

geoms::Mesh::Mesh(
  Vec o,
  std::string name,
  const Material* m,
  const AreaLight* l
) : Geom(m, l), points(), faces(), origin(o) {
  readPolyModel(name);
}

geoms::Mesh::Mesh(const Node& n)
  : Mesh(n.getVec("origin"), n.getString("file"),
         n.getMaterial("mat"), n.getLight("light")) {}

void geoms::Mesh::readPolyModel(std::string name) {
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
    throw std::runtime_error(importer.GetErrorString());
  }

  // Now we can access the file's contents.
  if (scene->mNumMeshes > 0) {
    // Process first mesh only right now.
    // TODO: process multiple meshes.
    aiMesh* mesh = scene->mMeshes[0];

    if (!mesh->HasPositions()) {
      throw std::runtime_error("No vertex positions on the mesh");
    }

    if (!mesh->HasNormals()) {
      throw std::runtime_error("No vertex normals on the mesh");
    }

    // Add points.
    for (size_t i = 0; i < mesh->mNumVertices; ++i) {
      aiVector3D thisPos = mesh->mVertices[i];
      aiVector3D thisNorm = mesh->mNormals[i];

      geoms::Poly::Point thisPoint;
      thisPoint.position = Vec(thisPos.x, thisPos.y, thisPos.z) + origin;
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
          ID(face.mIndices[0]),
          ID(face.mIndices[1]),
          ID(face.mIndices[2]),
          &points,
          mat,
          light
        );

        faces.push_back(thisPoly);
      }
    }
  }
}

bool geoms::Mesh::intersect(
  const Ray& /* r */,
  Intersection* /* isectOut */
) const {
  return debug::shouldNotReach(false);
}

bool geoms::Mesh::intersectShadow(
  const Ray& /* r */,
  float /* maxDist */
) const {
  return debug::shouldNotReach(false);
}

BBox geoms::Mesh::bounds() const {
  return debug::shouldNotReach(BBox());
}

void geoms::Mesh::refine(std::vector<const Geom*>& refined) const {
  for (const Poly& p : faces) {
    refined.push_back(&p);
  }
}

void geoms::Mesh::embreeIntersectCallback(
  const Embree::EmbreeObj* eo,
  const RTCRay& ray,
  Intersection* isectOut
) {
  float u = ray.u;
  float v = ray.v;
  float w = 1.0f - u - v;
  const geoms::Mesh* mesh = reinterpret_cast<const geoms::Mesh*>(eo->geom);
  const geoms::Poly& p = mesh->getFaces()[size_t(ray.primID)];
  
  isectOut->position = Vec(
    ray.org[0] + ray.dir[0] * ray.tfar,
    ray.org[1] + ray.dir[1] * ray.tfar,
    ray.org[2] + ray.dir[2] * ray.tfar
  );
  isectOut->distance = ray.tfar;
  isectOut->normal = (
    p.getPt0().normal * w + p.getPt1().normal * u + p.getPt2().normal * v
  ).normalized();
}

void geoms::Mesh::makeEmbreeObject(RTCScene scene, Embree::EmbreeObj& eo) const {
  unsigned geomId = rtcNewTriangleMesh(
    scene,
    RTC_GEOMETRY_STATIC,
    faces.size(),
    points.size()
  );

  Embree::EmbreeVert* vertices = reinterpret_cast<Embree::EmbreeVert*>(
    rtcMapBuffer(scene, geomId, RTC_VERTEX_BUFFER)
  );
  for (size_t i = 0; i < points.size(); ++i) {
    const Poly::Point& pt = points[i];
    vertices[i].x = pt.position.x();
    vertices[i].y = pt.position.y();
    vertices[i].z = pt.position.z();
    vertices[i].a = 0.0f;
  }
  rtcUnmapBuffer(scene, geomId, RTC_VERTEX_BUFFER);

  Embree::EmbreeTri* triangles =  reinterpret_cast<Embree::EmbreeTri*>(
    rtcMapBuffer(scene, geomId, RTC_INDEX_BUFFER)
  );
  for (size_t i = 0; i < faces.size(); ++i) {
    const Poly& p = faces[i];
    triangles[i].v0 = int(p.pt0.val);
    triangles[i].v1 = int(p.pt1.val);
    triangles[i].v2 = int(p.pt2.val);
  }
  rtcUnmapBuffer(scene, geomId, RTC_INDEX_BUFFER);

  eo = Embree::EmbreeObj(this, geomId, &geoms::Mesh::embreeIntersectCallback);
}
