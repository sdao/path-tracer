#include "parser.h"
#include <boost/format.hpp>
#include "scene.h"
#include "debug.h"

using boost::property_tree::ptree;
using boost::format;

Parser::Parser(const Scene& sc, const ptree& attr)
  : scene(sc), attributes(attr) {}

template<typename T>
T Parser::getItemFromStorage(
  const std::map<std::string, T>& storage,
  std::string key,
  bool allowNull
) const {
  std::string name = getString(key, allowNull);

  if (name.length() == 0) {
    return nullptr;
  } else if (storage.count(name) != 0) {
    return storage.at(name);
  } else {
    throw std::runtime_error(
      str(format("Property '%1%' references an unknown object") % key)
    );
  }
}

template<typename T>
std::vector<T> Parser::getItemsFromStorage(
  const std::map<std::string, T>& storage,
  std::string key
) const {
  const auto& optionalChildren = attributes.get_child_optional(key);

  if (!optionalChildren) {
    throw std::runtime_error(
      str(format("Cannot read object list property '%1%'") % key)
    );
  }

  std::vector<T> result;
  int i = 0;
  for (const auto& child : *optionalChildren) {
    const auto optionalName = child.second.get_value_optional<std::string>();

    if (!optionalName || storage.count(*optionalName) == 0) {
      throw std::runtime_error(
        str(format("Property '%1%' references an unknown object at index %2%")
          % key % i)
      );
    }

    result.push_back(storage.at(*optionalName));
    i++;
  }

  return result;
}

std::string Parser::getString(
  std::string key,
  bool allowEmpty
) const {
  const auto optionalString = attributes.get_optional<std::string>(key);

  if (!optionalString) {
    throw std::runtime_error(
      str(format("Required property '%1%' is missing") % key)
    );
  } else if (!allowEmpty && optionalString->length() == 0) {
    throw std::runtime_error(
      str(format("Property '%1%' must not be empty") % key)
    );
  }

  return *optionalString;
}

float Parser::getFloat(
  std::string key
) const {
  const auto optionalFloat = attributes.get_optional<float>(key);

  if (!optionalFloat) {
    throw std::runtime_error(
      str(format("Cannot read float property '%1%'") % key)
    );
  }

  return *optionalFloat;
}

int Parser::getInt(
  std::string key
) const {
  const auto optionalInt = attributes.get_optional<int>(key);

  if (!optionalInt) {
    throw std::runtime_error(
      str(format("Cannot read int property '%1%'") % key)
    );
  }

  return *optionalInt;
}

Vec Parser::getVec(std::string key) const {
  const auto optionalArray = attributes.get_child_optional(key);

  if (!optionalArray) {
    throw std::runtime_error(
      str(format("Cannot read vector property '%1%'") % key)
    );
  } else if (optionalArray->size() != 3) {
    throw std::runtime_error(
      str(format("Property '%1%' must have exactly 3 components") % key)
    );
  }

  Vec result;
  int i = 0;
  for (const auto& component : *optionalArray) {
    assert(i < 3);
    const auto optionalVal = component.second.get_value_optional<float>();

    if (!optionalVal) {
      throw std::runtime_error(
        str(format("Cannot read index %1% of vector '%2%'") % i % key)
      );
    }

    result[i] = *optionalVal;
    i++;
  }

  return result;
}

Transform Parser::getTransform(std::string key) const {
  Transform translation = math::translation(getVec(key + ".translate"));
  Transform rotation = math::angleAxisRotation(
    getFloat(key + ".rotate.angle"),
    getVec(key + ".rotate.axis")
  );

  return translation * rotation;
}

const AreaLight* Parser::getLight(std::string key) const {
  return getItemFromStorage<const AreaLight*>(scene.allLights, key, true);
}

const Material* Parser::getMaterial(std::string key) const {
  return getItemFromStorage<const Material*>(scene.allMaterials, key, true);
}

const Geom* Parser::getGeometry(std::string key) const {
  return getItemFromStorage<const Geom*>(scene.allGeometry, key, false);
}

std::vector<const Geom*> Parser::getGeometryList(std::string key) const {
  return getItemsFromStorage(scene.allGeometry, key);
}
