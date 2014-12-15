#pragma once
#include <OpenEXR/ImfRgbaFile.h>
#include <OpenEXR/ImfArray.h>
#include <vector>
#include <random>
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

  std::mt19937 rng;
  std::uniform_int_distribution<int> intDist;
  std::uniform_real_distribution<float> realDist;

  int iters;
  std::vector<std::vector<dvec>> data;
  Imf::Array2D<Imf::Rgba> exrData;

  geomptr intersect(const ray& r,
    std::vector<geomptr>& objs,
    intersection* isect_out);

public:
  camera(ray e, int ww, int hh, float ff);
  void renderOnce(std::vector<geomptr>& objs, std::string name);
  void renderInfinite(std::vector<geomptr>& objs, std::string name);
};
