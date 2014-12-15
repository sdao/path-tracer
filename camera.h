#pragma once
#include <OpenEXR/ImfRgbaFile.h>
#include <OpenEXR/ImfArray.h>
#include <tbb/tbb.h>
#include <vector>
#include "math.h"
#include "geom.h"

class camera {
  ray eye;
  int w;
  int h;
  float fovx2;

  vec up;
  vec right;
  vec corner_ray;

  tbb::mutex masterRngMutex;
  randomness masterRng;

  int iters;
  std::vector<std::vector<dvec>> data;
  Imf::Array2D<Imf::Rgba> exrData;

  geomptr intersect(const ray& r,
    const std::vector<geomptr>& objs,
    intersection* isect_out) const;

public:
  camera(ray e, int ww, int hh, float ff);
  void renderOnce(const std::vector<geomptr>& objs, std::string name);
  void renderInfinite(const std::vector<geomptr>& objs, std::string name);
};
