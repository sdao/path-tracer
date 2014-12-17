#pragma once
#include <OpenEXR/ImfRgbaFile.h>
#include <OpenEXR/ImfArray.h>
#include <tbb/tbb.h>
#include <vector>
#include "math.h"
#include "geom.h"
#include "kdtree.h"

class camera {
  static constexpr int MAX_DEPTH = 50; // Should be high enough to prevent bias.
  static constexpr int RUSSIAN_ROULETTE_DEPTH = 5;
  static constexpr int SAMPLES_PER_PIXEL = 4;
  static constexpr float PIXELS_PER_SAMPLE = 1.0f / float(SAMPLES_PER_PIXEL);

  ray eye;
  size_t w;
  size_t h;
  float fovx2;

  vec up;
  vec right;
  vec cornerRay;

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
