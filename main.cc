#include <vector>
#include <iostream>
#include <sstream>
#include "materials/all.h"
#include "geoms/all.h"
#include "light.h"
#include "camera.h"
#include "kdtree.h"
#include "scene.h"
#include "debug.h"

int main(int argc, char* argv[]) {
  int iterations = -1;

  if (argc >= 2) {
    std::istringstream itersArg(argv[1]);
    itersArg >> iterations;
  }

  try {
    Scene scene("scenes/spheres_scene.json");
    scene.allCameras.at("default")->renderMultiple("output.exr", iterations);
  } catch (std::exception& e) {
    debug::printNestedException(e);
    return 42;
  }

  return 0;
}
