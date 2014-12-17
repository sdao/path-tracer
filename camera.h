#pragma once
#include <OpenEXR/ImfRgbaFile.h>
#include <OpenEXR/ImfArray.h>
#include <tbb/tbb.h>
#include <vector>
#include "math.h"
#include "geom.h"
#include "kdtree.h"

class camera {
  ray eye;
  size_t w;
  size_t h;
  float fovx2;

  vec up;
  vec right;
  vec corner_ray;

  randomness masterRng;
  std::vector<unsigned> rowSeeds;

  std::vector< std::vector<dvec> > data;
  Imf::Array2D<Imf::Rgba> exrData;

  int iters;

public:
  camera(ray e, size_t ww, size_t hh, float ff = float(M_PI_4));
  void renderOnce(const kdtree& kdt, std::string name);
  [[noreturn]] void renderInfinite(
    const kdtree& kdt,
    std::string name
  );
};
