#pragma once
#include <map>
#include <boost/property_tree/ptree.hpp>
#include "core.h"

class AreaLight;
class Material;
class Geom;

class Parser {
  const std::map<std::string, const AreaLight*>& lights;
  const std::map<std::string, const Material*>& materials;
  const std::map<std::string, const Geom*>& geometry;
  const boost::property_tree::ptree& attributes;

private:
  template<typename T>
  T getRefAny(
    const std::map<std::string, T>& storage,
    std::string key,
    bool allowNull
  ) const;

public:
  Parser(
    const std::map<std::string, const AreaLight*>& sceneLights,
    const std::map<std::string, const Material*>& sceneMats,
    const std::map<std::string, const Geom*>& sceneGeoms,
    const boost::property_tree::ptree& attr
  );

  std::string getString(
    std::string key,
    bool allowEmpty
  ) const;

  float getFloat(
    std::string key
  ) const;

  int getInt(
    std::string key
  ) const;
  
  Vec getVec(std::string key) const;

  Transform getTransform(std::string key) const;

  const AreaLight* getLight(std::string key) const;
  const Material* getMaterial(std::string key) const;
  const Geom* getGeom(std::string key) const;
};
