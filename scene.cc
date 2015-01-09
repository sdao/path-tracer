#include "scene.h"
#include <exception>
#include "camera.h"
#include "light.h"
#include "material.h"
#include "geom.h"
#include "materials/all.h"
#include "geoms/all.h"
#include "kdtree.h"

using namespace boost::property_tree;

Scene::Scene(std::string jsonFile)
  : sceneLights(), sceneMats(), sceneGeoms(), camera(nullptr)
{
  try {
    ptree pt;
    read_json(jsonFile, pt);

    readLights(pt);
    readMats(pt);
    readGeoms(pt);
    readCamera(pt);
  } catch (const std::exception&) {
    cleanUp();
    throw;
  }
}

Scene::~Scene() {
  cleanUp();
}

void Scene::cleanUp() {
  for (auto& pair : sceneLights) {
    delete pair.second;
  }

  for (auto& pair : sceneMats) {
    delete pair.second;
  }

  for (auto& pair : sceneGeoms) {
    delete pair.second;
  }

  delete camera;
}

template<typename T>
void Scene::readAny(
  const boost::property_tree::ptree& root,
  const std::string& prefix,
  std::map<std::string, T>& storage,
  std::function<T(const Parser&)> readSingleFunc
) {
  const auto& children = root.get_child(prefix);

  int count = 0;
  for (const auto& child : children) {
    const std::string name = child.first;
    const auto& attr = child.second;
    Parser parser(sceneLights, sceneMats, sceneGeoms, attr);

    try {
      if (name.length() == 0) {
        throw std::runtime_error("No name");
      } else if (storage.count(name) > 0) {
        throw std::runtime_error("Name was reused");
      }

      storage[name] = readSingleFunc(parser);
    } catch (std::exception& e) {
      std::stringstream msg;
      msg << prefix << ".[" << count << "]" << name << ": " << e.what();
      throw std::runtime_error(msg.str());
    }

    count++;
  }
}

void Scene::readLights(const ptree& root) {
  readAny<const AreaLight*>(root, "lights", sceneLights, [](const Parser& p) -> const AreaLight* {
    const std::string type = p.getString("type", false);

    if (type == "area") {
      return new AreaLight(p);
    } else {
      throw std::runtime_error(type + " is not a recognized light type");
    }
  });
}

void Scene::readMats(const ptree& root) {
  readAny<const Material*>(root, "materials", sceneMats, [](const Parser& p) -> const Material* {
    const std::string type = p.getString("type", false);

    if (type == "dielectric") {
      return new materials::Dielectric(p);
    } else if (type == "lambert") {
      return new materials::Lambert(p);
    } else if (type == "phong") {
      return new materials::Phong(p);
    } else {
      throw std::runtime_error(type + " is not a recognized material type");
    }
  });
}

void Scene::readGeoms(const ptree& root) {
  readAny<const Geom*>(root, "geometry", sceneGeoms, [&](const Parser& p) -> const Geom* {
    const std::string type = p.getString("type", false);

    if (type == "disc") {
      return new geoms::Disc(p);
    } else if (type == "sphere") {
      return new geoms::Sphere(p);
    } else if (type == "mesh") {
      return new geoms::Mesh(p);
    } else if (type == "inverted") {
      return new geoms::Inverted(p);
    } else {
      throw std::runtime_error(type + " is not a recognized geometry type");
    }
  });
}

void Scene::readCamera(const boost::property_tree::ptree& root) {
  const auto& attr = root.get_child("camera");
  Parser parser(sceneLights, sceneMats, sceneGeoms, attr);

  camera = new Camera(parser);
}

void Scene::renderMultiple(
  std::string name,
  int iterations
) {
  KDTree tree(sceneGeoms);
  tree.build();
  camera->renderMultiple(tree, name, iterations);
}
