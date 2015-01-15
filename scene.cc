#include "scene.h"
#include <exception>
#include <boost/format.hpp>
#include "camera.h"
#include "light.h"
#include "material.h"
#include "geom.h"
#include "materials/all.h"
#include "geoms/all.h"
#include "node.h"

using boost::property_tree::ptree;
using boost::format;

Scene::Scene(std::string jsonFile)
  : lights(), materials(), geometry(), cameras()
{
  try {
    ptree pt;
    read_json(jsonFile, pt);

    readLights(pt);
    readMats(pt);
    readGeoms(pt);
    readCameras(pt);
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

  for (auto& pair : cameras) {
    delete pair.second;
  }
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
      const Node node(child.second, *this);
      const std::string type = node.getString("type");

      if (name.length() == 0) {
        throw std::runtime_error("No name");
      } else if (storage.count(name) != 0) {
        throw std::runtime_error("Name was reused");
      } else if (lookup.count(type) == 0) {
        throw std::runtime_error(type + " is not a recognized type");
      }

      storage[name] = lookup.at(type)(node);
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
    { "area", [](const Node& n) { return new AreaLight(n); } }
  };

  readMultiple<const AreaLight*>(root, "lights", lightLookup, lights);
}

void Scene::readMats(const ptree& root) {
  using namespace materials;
  static const LookupMap<const Material*> materialLookup = {
    { "dielectric", [](const Node& n) { return new Dielectric(n); } },
    { "lambert",    [](const Node& n) { return new Lambert(n); } },
    { "phong",      [](const Node& n) { return new Phong(n); } }
  };

  readMultiple<const Material*>(root, "materials", materialLookup, materials);
}

void Scene::readGeoms(const ptree& root) {
  using namespace geoms;
  static const LookupMap<const Geom*> geometryLookup = {
    { "disc",     [](const Node& n) { return new Disc(n); } },
    { "sphere",   [](const Node& n) { return new Sphere(n); } },
    { "mesh",     [](const Node& n) { return new Mesh(n); } }
  };

  readMultiple<const Geom*>(root, "geometry", geometryLookup, geometry);
}

void Scene::readCameras(const ptree& root) {
  static const LookupMap<Camera*> cameraLookup = {
    { "persp", [](const Node& n) { return new Camera(n); } }
  };

  readMultiple<Camera*>(root, "cameras", cameraLookup, cameras);
}

Camera* Scene::defaultCamera() const {
  if (cameras.count("default") == 0) {
    throw std::runtime_error("Scene contains no default camera");
  }

  return cameras.at("default");
}
