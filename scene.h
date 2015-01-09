#pragma once
#include <vector>
#include <sstream>
#include <functional>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "parser.h"

class AreaLight;
class Material;
class Geom;
class Camera;

class Scene {
  std::map<std::string, const AreaLight*> sceneLights;
  std::map<std::string, const Material*> sceneMats;
  std::map<std::string, const Geom*> sceneGeoms;
  Camera* camera;

  void cleanUp();

  template<typename T>
  inline void readAny(
    const boost::property_tree::ptree& root,
    const std::string& prefix,
    std::map<std::string, T>& storage,
    std::function<T(const Parser&)> readSingleFunc
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
  void renderMultiple(
    std::string name,
    int iterations
  );
};
