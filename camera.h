#pragma once
#include <OpenEXR/ImfRgbaFile.h>
#include <OpenEXR/ImfArray.h>
#include <tbb/tbb.h>
#include <vector>
#include "core.h"
#include "geom.h"
#include "kdtree.h"

/**
 * Manages rendering by simulating the action of a physical pinhole camera.
 */
class camera {
  /**
   * The maximum amount of "bounces" a ray can take.
   * This amount is used to prevent pathological cases from causing a ray to
   * infinitely bounce. It should be high enough to prevent sampling bias.
   */
  static constexpr int MAX_DEPTH = 50;
  /**
   * The number of bounces at which a ray is subject to Russian Roulette
   * termination.
   */
  static constexpr int RUSSIAN_ROULETTE_DEPTH = 5;
  /**
   * The number of samples to take per pixel in each iteration (supersampling).
   */
  static constexpr int SAMPLES_PER_PIXEL = 4;
  /**
   * The inverse of SAMPLES_PER_PIXEL.
   */
  static constexpr float PIXELS_PER_SAMPLE = 1.0f / float(SAMPLES_PER_PIXEL);

  ray eye; /**< The ray representing the eye's position and orientation. */
  float fovx2; /**< Half of the horizontal field of view angle. */

  vec up; /**< The ray increment moving up in the y-direction. */
  vec right; /**< The ray increment moving right in the x-direction. */
  vec cornerRay; /**< The first ray in the lower-left corner. */

  randomness masterRng; /**< The RNG used to seed the per-row RNGs. */
  std::vector<unsigned> rowSeeds; /**< The per-row RNG seeds. */

  std::vector< std::vector<dvec> > data; /**< The colors sampled from paths. */
  Imf::Array2D<Imf::Rgba> exrData; /**< The colors converted to OpenEXR. */

  size_t w; /**< The width of the output image to generate. */
  size_t h; /**< The height of the output image to generate. */
  int iters; /** The current number of path-tracing iterations done. */

public:
  /**
   * Constructs a camera.
   *
   * @param e  a ray specifying the eye's position and orientation
   * @param ww the width of the output image, in pixels
   * @param hh the height of the output image, in pixels
   * @param ff the total horizontal field of view, in radians
   */
  camera(ray e, size_t ww, size_t hh, float ff = math::PI_4);

  /**
   * Renders an additional iteration of the image by path-tracing.
   * If there are existing iterations, the additional iteration will be
   * combined in a weighted manner.
   *
   * @param kdt  a k-d tree containing the scene's geometry
   * @param name the name of the output EXR file
   */
  void renderOnce(const kdtree& kdt, std::string name);

  /**
   * Renders multiple additional path-tracing iterations.
   * To render infinite iterations, specify iterations = -1.
   *
   * @param kdt        a k-d tree containing the scene's geometry
   * @param name       the name of the output EXR file
   * @param iterations the number of iterations to render; if < 0, then this
   *                   function will run forever
   */
  void renderMultiple(
    const kdtree& kdt,
    std::string name,
    int iterations
  );
};
