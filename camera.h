#pragma once
#include <OpenEXR/ImfRgbaFile.h>
#include <OpenEXR/ImfArray.h>
#include <tbb/tbb.h>
#include <vector>
#include "math.h"
#include "geom.h"

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

  geomptr intersect(
    const ray& r,
    const std::vector<geomptr>& objs,
    intersection* isect_out = nullptr
  ) const;

public:
  camera(ray e, size_t ww, size_t hh, float ff = float(M_PI_4));
  void renderOnce(const std::vector<geomptr>& objs, std::string name);
  [[noreturn]] void renderInfinite(
    const std::vector<geomptr>& objs,
    std::string name
  );
};
