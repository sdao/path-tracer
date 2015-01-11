#pragma once
#include <vector>
#include <map>
#include <functional>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "core.h"

class AreaLight;
class Material;
class Geom;
class Camera;
class Node;

class Scene {
  friend class Node;

  /** A table mapping strings to functions that create objects from raw data. */
  template <typename T>
  using LookupMap = std::map< std::string, std::function<T(const Node&)> >;

  /** Lights read from scene file. */
  std::map<std::string, const AreaLight*> lights;
  /** Materials read from scene file. */
  std::map<std::string, const Material*> materials;
  /** Geometry read from file. */
  std::map<std::string, const Geom*> geometry;
  /** Cameras read from file. */
  std::map<std::string, Camera*> cameras;

  /** Deletes all owned dynamically-allocated objects. */
  void cleanUp();

  /**
   * Reads multiple objects stored in a property tree.
   *
   * @param root    the root of the property tree
   * @param prefix  the path in the property tree that contains the objects
   * @param lookup  the lookup table used to construct objects from
   *                the raw data in the property tree
   * @param storage the map in which to place the read objects
   */
  template<typename T>
  void readMultiple(
    const boost::property_tree::ptree& root,
    const std::string& prefix,
    const LookupMap<T> lookup,
    std::map<std::string, T>& storage
  );
  /** Reads all of the lights in the given property tree. */
  void readLights(const boost::property_tree::ptree& root);
  /** Reads all of the materials in the given property tree. */
  void readMats(const boost::property_tree::ptree& root);
  /** Reads all of the geometry in the given property tree. */
  void readGeoms(const boost::property_tree::ptree& root);
  /** Reads all of the cameras in the given property tree. */
  void readCameras(const boost::property_tree::ptree& root);
  
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
   * Returns the camera named "default" from the scene.
   *
   * @throws std::runtime_error if there is no camera named "default"
   */
  Camera* defaultCamera() const;
};
