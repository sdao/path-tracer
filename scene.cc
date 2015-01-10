#include "scene.h"
#include <exception>
#include <boost/format.hpp>
#include "camera.h"
#include "light.h"
#include "material.h"
#include "geom.h"
#include "materials/all.h"
#include "geoms/all.h"
#include "kdtree.h"

using boost::property_tree::ptree;
using boost::format;

Scene::Scene(std::string jsonFile)
  : lights(), materials(), geometry(), camera(nullptr)
{
  try {
    ptree pt;
    read_json(jsonFile, pt);

    readLights(pt);
    readMats(pt);
    readGeoms(pt);
    readCamera(pt);
  } catch (...) {
    cleanUp();
    throw;
  }
}

Scene::~Scene() {
  cleanUp();
}

void Scene::cleanUp() {
  for (auto& pair : lights) {
    delete pair.second;
  }

  for (auto& pair : materials) {
    delete pair.second;
  }

  for (auto& pair : geometry) {
    delete pair.second;
  }

  delete camera;
}

template<typename T>
void Scene::readMultiple(
  const boost::property_tree::ptree& root,
  const std::string& prefix,
  const LookupMap<T> lookup,
  std::map<std::string, T>& storage
) {
  const auto& children = root.get_child(prefix);

  int count = 0;
  for (const auto& child : children) {
    const std::string name = child.first;

    try {
      const Parser parser(lights, materials, geometry, child.second);
      const std::string type = parser.getString("type", false);

      if (name.length() == 0) {
        throw std::runtime_error("No name");
      } else if (storage.count(name) > 0) {
        throw std::runtime_error("Name was reused");
      } else if (lookup.count(type) == 0) {
        throw std::runtime_error(type + " is not a recognized type");
      }

      storage[name] = lookup.at(type)(parser);
    } catch (...) {
      std::throw_with_nested(std::runtime_error(
        str(format("Error parsing node (%1%.[%2%]%3%)") % prefix % count % name)
      ));
    }

    count++;
  }
}

void Scene::readLights(const ptree& root) {
  static const LookupMap<const AreaLight*> lightLookup = {
    { "area", [](const Parser& p) { return new AreaLight(p); } }
  };

  readMultiple<const AreaLight*>(root, "lights", lightLookup, lights);
}

void Scene::readMats(const ptree& root) {
  using namespace materials;
  static const LookupMap<const Material*> materialLookup = {
    { "dielectric", [](const Parser& p) { return new Dielectric(p); } },
    { "lambert",    [](const Parser& p) { return new Lambert(p); } },
    { "phong",      [](const Parser& p) { return new Phong(p); } }
  };

  readMultiple<const Material*>(root, "materials", materialLookup, materials);
}

void Scene::readGeoms(const ptree& root) {
  using namespace geoms;
  static const LookupMap<const Geom*> geometryLookup = {
    { "disc",     [](const Parser& p) { return new Disc(p); } },
    { "inverted", [](const Parser& p) { return new Inverted(p); } },
    { "sphere",   [](const Parser& p) { return new Sphere(p); } },
    { "mesh",     [](const Parser& p) { return new Mesh(p); } }
  };

  readMultiple<const Geom*>(root, "geometry", geometryLookup, geometry);
}

void Scene::readCamera(const ptree& root) {
  const auto& attr = root.get_child("camera");

  try {
    const Parser parser(lights, materials, geometry, attr);
    camera = new Camera(parser);
  } catch (...) {
    std::throw_with_nested(std::runtime_error(
      "Error parsing node (camera)"
    ));
  }
}

void Scene::renderMultiple(std::string name, int iterations) {
  KDTree tree(geometry);
  tree.build();
  camera->renderMultiple(tree, name, iterations);
}
