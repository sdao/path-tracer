#pragma once
#include <vector>
#include <map>
#include <boost/property_tree/ptree.hpp>
#include "core.h"

class AreaLight;
class Material;
class Geom;
class Scene;

/**
 * Wrapper class around a boost::property_tree::ptree to help
 * facilitate data access.
 */
class Node {
  struct NodeVecTranslator {
    typedef std::string internal_type;
    typedef Vec external_type;

    NodeVecTranslator();
    boost::optional<external_type> get_value(const internal_type& data) const;
  };

  template <typename T, bool allowNull = true>
  struct NodeLookupTranslator {
    typedef std::string internal_type;
    typedef T external_type;

    const std::map<internal_type, external_type>& lookup;
    NodeLookupTranslator(const std::map<internal_type, external_type>& l);
    boost::optional<external_type> get_value(const internal_type& data) const;
  };

  const boost::property_tree::ptree& attributes;
  const Scene& container;

public:
  Node(const boost::property_tree::ptree& attr, const Scene& cont);

  std::string getString(std::string key) const;
  int getInt(std::string key) const;
  float getFloat(std::string key) const;
  Vec getVec(std::string key) const;
  const AreaLight* getLight(std::string key) const;
  const Material* getMaterial(std::string key) const;
  const Geom* getGeometry(std::string key) const;
  std::vector<const Geom*> getGeometryList(std::string key) const;
  Transform getTransform(std::string key) const;
};
