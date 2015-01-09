#include "parser.h"

using namespace boost::property_tree;

Parser::Parser(
  const std::map<std::string, const AreaLight*>& sceneLights,
  const std::map<std::string, const Material*>& sceneMats,
  const std::map<std::string, const Geom*>& sceneGeoms,
  const ptree& attr
) : lights(sceneLights),
    materials(sceneMats),
    geometry(sceneGeoms),
    attributes(attr) {}

template<typename T>
T Parser::getRefAny(
  const std::map<std::string, T>& storage,
  std::string key,
  bool allowNull
) const {
  std::string name = getString(key, allowNull);

  if (name.length() == 0) {
    return nullptr;
  } else if (storage.count(name) > 0) {
    return storage.at(name);
  } else {
    throw std::runtime_error(key + " references a non-existent object");
  }
}

std::string Parser::getString(
  std::string key,
  bool allowEmpty
) const {
  const auto optionalString = attributes.get_optional<std::string>(key);
  if (!optionalString) {
    throw std::runtime_error(key + " is missing");
  }

  if (!allowEmpty && optionalString->length() == 0) {
    throw std::runtime_error(key + " must not be empty");
  }

  return *optionalString;
}

float Parser::getFloat(
  std::string key
) const {
  const auto optionalFloat = attributes.get_optional<float>(key);
  if (!optionalFloat) {
    throw std::runtime_error(key + " is missing or is not a float");
  }

  return *optionalFloat;
}

int Parser::getInt(
  std::string key
) const {
  const auto optionalInt = attributes.get_optional<int>(key);
  if (!optionalInt) {
    throw std::runtime_error(key + " is missing or is not an integer");
  }

  return *optionalInt;
}

Vec Parser::getVec(std::string key) const {
  const auto optionalArray = attributes.get_child_optional(key);
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

Transform Parser::getTransform(std::string key) const {
  Transform translation = math::translation(getVec(key + ".translate"));
  Transform rotation = math::angleAxisRotation(
    getFloat(key + ".rotate.angle"),
    getVec(key + ".rotate.axis")
  );

  return translation * rotation;
}

const AreaLight* Parser::getLight(std::string key) const {
  return getRefAny<const AreaLight*>(lights, key, true);
}

const Material* Parser::getMaterial(std::string key) const {
  return getRefAny<const Material*>(materials, key, true);
}

const Geom* Parser::getGeom(std::string key) const {
  return getRefAny<const Geom*>(geometry, key, false);
}
