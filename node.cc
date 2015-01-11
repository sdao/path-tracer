#include "node.h"
#include <boost/algorithm/string.hpp>
#include "scene.h"

Node::Node(const boost::property_tree::ptree& attr, const Scene& cont)
  : attributes(attr), container(cont) {}

std::string Node::getString(std::string key) const {
  return attributes.get<std::string>(key);
}

int Node::getInt(std::string key) const {
  return attributes.get<int>(key);
}

float Node::getFloat(std::string key) const {
  return attributes.get<float>(key);
}

Vec Node::getVec(std::string key) const {
  const NodeVecTranslator t;
  return attributes.get<Vec, NodeVecTranslator>(key, t);
}

const AreaLight* Node::getLight(std::string key) const {
  using NodeLightTranslator = Node::NodeLookupTranslator<const AreaLight*>;
  const NodeLightTranslator t(container.lights);
  return attributes.get<const AreaLight*, NodeLightTranslator>(key, t);
}

const Material* Node::getMaterial(std::string key) const {
  using NodeMaterialTranslator = Node::NodeLookupTranslator<const Material*>;
  const NodeMaterialTranslator t(container.materials);
  return attributes.get<const Material*, NodeMaterialTranslator>(key, t);
}

const Geom* Node::getGeometry(std::string key) const {
  using NodeGeometryTranslator = Node::NodeLookupTranslator<const Geom*>;
  const NodeGeometryTranslator t(container.geometry);
  return attributes.get<const Geom*, NodeGeometryTranslator>(key, t);
}

std::vector<const Geom*> Node::getGeometryList(std::string key) const {
  using NodeGeometryTranslator = Node::NodeLookupTranslator<const Geom*, false>;
  const NodeGeometryTranslator t(container.geometry);

  const auto& listRoot = attributes.get_child(key);

  std::vector<const Geom*> result;
  for (const auto& listItem : listRoot) {
    const Geom* item = listItem.second.get_value<const Geom*>(t);
    result.push_back(item);
  }

  return result;
}

Transform Node::getTransform(std::string key) const {
  Transform translation = math::translation(getVec(key + ".translate"));
  Transform rotation = math::angleAxisRotation(
    getFloat(key + ".rotate.angle"),
    getVec(key + ".rotate.axis")
  );

  return translation * rotation;
}

Node::NodeVecTranslator::NodeVecTranslator() {}

boost::optional<Vec> Node::NodeVecTranslator::get_value(
  const std::string& data
) const {;
  std::vector<std::string> tokens;
  boost::algorithm::split(
    tokens,
    data,
    boost::is_space(),
    boost::token_compress_on
  );

  if (tokens.size() != 3) {
    return boost::optional<Vec>();
  }

  Vec result;
  try {
    result[0] = std::stof(tokens[0]);
    result[1] = std::stof(tokens[1]);
    result[2] = std::stof(tokens[2]);
  } catch (...) {
    return boost::optional<Vec>();
  }

  return boost::optional<Vec>(result);
}

template <typename T, bool allowNull>
Node::NodeLookupTranslator<T, allowNull>::NodeLookupTranslator(
  const std::map<std::string, T>& l
) : lookup(l) {}

template <typename T, bool allowNull>
boost::optional<T> Node::NodeLookupTranslator<T, allowNull>::get_value(
  const std::string& data
) const {
  if (data.length() == 0 && allowNull) {
    return boost::optional<T>(nullptr);
  } else if (lookup.count(data) != 0) {
    return boost::optional<T>(lookup.at(data));
  }

  return boost::optional<T>();
}
