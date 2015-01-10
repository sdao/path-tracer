#include "parser.h"
#include <boost/format.hpp>

using boost::property_tree::ptree;
using boost::format;

Parser::Parser(
  const std::map<std::string, const AreaLight*>& ll,
  const std::map<std::string, const Material*>& mm,
  const std::map<std::string, const Geom*>& gg,
  const ptree& attr
) : lights(ll), materials(mm), geometry(gg), attributes(attr) {}

template<typename T>
T Parser::getItemFromStorage(
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
    throw std::runtime_error(
      str(format("Property '%1%' references an unknown object") % key)
    );
  }
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
  }

  if (!allowEmpty && optionalString->length() == 0) {
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
  }

  Vec result;
  int count = 0;
  for (const auto& component : *optionalArray) {
    const auto optionalVal = component.second.get_value_optional<float>();

    if (count < 3) {
      if (!optionalVal) {
        throw std::runtime_error(
          str(format("Cannot read index %1% of vector '%2%'") % count % key)
        );
      }

      result[count] = *optionalVal;
    }

    count++;
  }

  if (count != 3) {
    throw std::runtime_error(
      str(format("Property '%1%' must have exactly 3 components") % key)
    );
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
  return getItemFromStorage<const AreaLight*>(lights, key, true);
}

const Material* Parser::getMaterial(std::string key) const {
  return getItemFromStorage<const Material*>(materials, key, true);
}

const Geom* Parser::getGeom(std::string key) const {
  return getItemFromStorage<const Geom*>(geometry, key, false);
}
