#pragma once
#include <vector>
#include <map>
#include <functional>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "parser.h"

class AreaLight;
class Material;
class Geom;
class Camera;

class Scene {
  template <typename T>
  using LookupMap = std::map< std::string, std::function<T(const Parser&)> >;

  std::map<std::string, const AreaLight*> lights;
  std::map<std::string, const Material*> materials;
  std::map<std::string, const Geom*> geometry;
  Camera* camera;

  void cleanUp();

  template<typename T>
  void readMultiple(
    const boost::property_tree::ptree& root,
    const std::string& prefix,
    const LookupMap<T> lookup,
    std::map<std::string, T>& storage
  );
  void readLights(const boost::property_tree::ptree& root);
  void readMats(const boost::property_tree::ptree& root);
  void readGeoms(const boost::property_tree::ptree& root);
  void readCamera(const boost::property_tree::ptree& root);
  
public:
  /**
   * Constructs a scene by reading it from a JSON scene description.
   *
   * @param jsonFile the name of the JSON file to read
   *
   * @throws std::exception if the scene could not be created from the JSON file
   */
  Scene(std::string jsonFile);
  ~Scene();

  /**
   * Renders multiple additional path-tracing iterations.
   * To render infinite iterations, specify iterations = -1.
   *
   * @param name       the name of the output EXR file
   * @param iterations the number of iterations to render; if < 0, then this
   *                   function will run forever
   */
  void renderMultiple(std::string name, int iterations);
};
