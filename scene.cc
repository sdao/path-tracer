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
  std::function<T(const boost::property_tree::ptree&)> readSingleFunc
) {
  const auto& children = root.get_child(prefix);

  int count = 0;
  for (const auto& child : children) {
    const std::string name = child.first;
    const auto& attr = child.second;

    try {
      if (name.length() == 0) {
        throw std::runtime_error("No name");
      } else if (storage.count(name) > 0) {
        throw std::runtime_error("Name was reused");
      }

      storage[name] = readSingleFunc(attr);
    } catch (std::exception& e) {
      std::stringstream msg;
      msg << prefix << ".[" << count << "]" << name << ": " << e.what();
      throw std::runtime_error(msg.str());
    }

    count++;
  }
}

void Scene::readLights(const ptree& root) {
  readAny<const AreaLight*>(root, "lights", sceneLights, [](const ptree& attr) {
    const std::string type = parseString(attr, "type", false);
    const AreaLight* light = nullptr;

    if (type == "area") {
      Vec color = parseVec(attr, "color");
      light = new AreaLight(color);
    } else {
      throw std::runtime_error(type + " is not a recognized light type");
    }

    return light;
  });
}

void Scene::readMats(const ptree& root) {
  readAny<const Material*>(root, "materials", sceneMats, [](const ptree& attr) {
    const std::string type = parseString(attr, "type", false);
    const Material* mat = nullptr;

    if (type == "dielectric") {
      Vec color = parseVec(attr, "color");
      float ior = parseFloat(attr, "ior");
      mat = new materials::Dielectric(ior, color);
    } else if (type == "lambert") {
      Vec albedo = parseVec(attr, "albedo");
      mat = new materials::Lambert(albedo);
    } else if (type == "phong") {
      Vec color = parseVec(attr, "color");
      float exponent = parseFloat(attr, "exponent");
      mat = new materials::Phong(exponent, color);
    } else {
      throw std::runtime_error(type + " is not a recognized material type");
    }

    return mat;
  });
}

void Scene::readGeoms(const ptree& root) {
  readAny<const Geom*>(root, "geometry", sceneGeoms, [&](const ptree& attr) {
    const std::string type = parseString(attr, "type", false);
    const Geom* geom = nullptr;

    if (type == "disc") {
      Vec origin = parseVec(attr, "origin");
      Vec normal = parseVec(attr, "normal");
      float radiusOuter = parseFloat(attr, "radiusOuter");
      float radiusInner = parseFloat(attr, "radiusInner");
      const Material* material = parseRefAny(attr, sceneMats, "material", true);
      const AreaLight* light = parseRefAny(attr, sceneLights, "light", true);
      geom = new geoms::Disc(
        origin, normal, radiusOuter, radiusInner, material, light
      );
    } else if (type == "sphere") {
      Vec origin = parseVec(attr, "origin");
      float radius = parseFloat(attr, "radius");
      const Material* material = parseRefAny(attr, sceneMats, "material", true);
      const AreaLight* light = parseRefAny(attr, sceneLights, "light", true);
      geom = new geoms::Sphere(origin, radius, material, light);
    } else if (type == "mesh") {
      Vec origin = parseVec(attr, "origin");
      std::string file = parseString(attr, "file", false);
      const Material* material = parseRefAny(attr, sceneMats, "material", true);
      const AreaLight* light = parseRefAny(attr, sceneLights, "light", true);
      geom = new geoms::Mesh(origin, file, material, light);
    } else if (type == "inverted") {
      const Geom* obj = parseRefAny(attr, sceneGeoms, "geometry", false);
      geom = new geoms::Inverted(obj);
    } else {
      throw std::runtime_error(type + " is not a recognized geometry type");
    }

    return geom;
  });
}

void Scene::readCamera(const boost::property_tree::ptree& root) {
  const auto& attr = root.get_child("camera");

  Vec translate = parseVec(attr, "translate");
  float angle = parseFloat(attr, "rotate.angle");
  Vec axis = parseVec(attr, "rotate.axis");
  int width = parseInt(attr, "width");
  int height = parseInt(attr, "height");
  float fov = parseFloat(attr, "fov");
  float focalLength = parseFloat(attr, "focalLength");
  float fStop = parseFloat(attr, "fStop");

  camera = new Camera(
    math::translation(translate) * math::angleAxisRotation(angle, axis),
    width,
    height,
    fov,
    focalLength,
    fStop
  );
}

template<typename T>
T Scene::parseRefAny(
  const boost::property_tree::ptree& attr,
  std::map<std::string, T>& storage,
  std::string key,
  bool allowNull
) {
  std::string name = parseString(attr, key, allowNull);

  if (name.length() == 0) {
    return nullptr;
  } else if (storage.count(name) > 0) {
    return storage[name];
  } else {
    throw std::runtime_error(key + " references a non-existent object");
  }
}

std::string Scene::parseString(
  const boost::property_tree::ptree& attr,
  std::string key,
  bool allowEmpty
) {
  const auto optionalString = attr.get_optional<std::string>(key);
  if (!optionalString) {
    throw std::runtime_error(key + " is missing");
  }

  if (!allowEmpty && optionalString->length() == 0) {
    throw std::runtime_error(key + " must not be empty");
  }

  return *optionalString;
}

float Scene::parseFloat(
  const boost::property_tree::ptree& attr,
  std::string key
) {
  const auto optionalFloat = attr.get_optional<float>(key);
  if (!optionalFloat) {
    throw std::runtime_error(key + " is missing or is not a float");
  }

  return *optionalFloat;
}

int Scene::parseInt(
  const boost::property_tree::ptree& attr,
  std::string key
) {
  const auto optionalInt = attr.get_optional<int>(key);
  if (!optionalInt) {
    throw std::runtime_error(key + " is missing or is not an integer");
  }

  return *optionalInt;
}

Vec Scene::parseVec(const ptree& attr, std::string key) {
  const auto optionalArray = attr.get_child_optional(key);
  if (!optionalArray) {
    throw std::runtime_error(key + " is missing");
  }

  Vec result;

  int count = 0;
  for (const auto& component : *optionalArray) {
    const auto optionalVal = component.second.get_value_optional<float>();

    if (count < 3) {
      if (!optionalVal) {
        throw std::runtime_error(
          key + " is invalid at component " + std::to_string(count)
        );
      }

      result[count] = *optionalVal;
    }

    count++;
  }

  if (count != 3) {
    throw std::runtime_error(key + " must have exactly 3 components");
  }

  return result;
}

void Scene::renderMultiple(
  std::string name,
  int iterations
) {
  KDTree tree(sceneGeoms);
  tree.build();
  camera->renderMultiple(tree, name, iterations);
}
