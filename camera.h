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
class Camera {
  /**
   * The number of bounces at which a ray is subject to Russian Roulette
   * termination, stage 1 (less aggressive).
   */
  static constexpr int RUSSIAN_ROULETTE_DEPTH_1 = 5;
  /**
   * The number of bounces at which a ray is subject to Russian Roulette
   * termination, stage 2 (more aggressive).
   */
  static constexpr int RUSSIAN_ROULETTE_DEPTH_2 = 50;
  /**
   * The number of samples to take per pixel in each iteration (supersampling).
   */
  static constexpr int SAMPLES_PER_PIXEL = 4;
  /**
   * Maximum positive or negative pixel offset of samples.
   */
  static constexpr float FILTER_WIDTH = 2.0f;

  Ray eye; /**< The ray representing the eye's position and orientation. */

  Vec up; /**< The ray increment moving up in the y-direction. */
  Vec right; /**< The ray increment moving right in the x-direction. */
  Vec cornerRay; /**< The first ray in the lower-left corner. */

  Randomness masterRng; /**< The RNG used to seed the per-row RNGs. */
  std::vector<unsigned> rowSeeds; /**< The per-row RNG seeds. */

  std::vector< std::vector<DoubleVec> > colors; /**< The raw sampled colors. */
  std::vector< std::vector<double> > weights; /**< The raw filter weights. */
  Imf::Array2D<Imf::Rgba> exrData; /**< The filtered image for OpenEXR. */

  const size_t w; /**< The width of the output image to generate. */
  const size_t h; /**< The height of the output image to generate. */
  int iters; /** The current number of path-tracing iterations done. */

  /**
   * Randomly picks a light and samples it for direct illumination. The
   * radiance returned will be scaled according to the probability of picking
   * the light.
   *
   * @param rng         the per-thread RNG in use
   * @param incoming    the ray coming into the intersection on the target
   *                    geometry (on the reflector)
   * @param isect       the intersection on the target geometry that should be
   *                    illuminated
   * @param mat         the material of the target geometry being illuminated
   * @param lights      a list of light-emitting objects to randomly pick from
   * @param kdt         the k-d tree containing all geometry in the scene
   */
  Vec uniformSampleOneLight(
    Randomness& rng,
    const LightRay& incoming,
    const Intersection& isect,
    const Material* mat,
    const std::vector<Geom*>& lights,
    const KDTree& kdt
  ) const;

public:
  /**
   * Constructs a camera.
   *
   * @param e  a ray specifying the eye's position and orientation
   * @param ww the width of the output image, in pixels
   * @param hh the height of the output image, in pixels
   * @param ff the total horizontal field of view, in radians
   */
  Camera(Ray e, size_t ww, size_t hh, float ff = math::PI_4);

  /**
   * Renders an additional iteration of the image by path-tracing.
   * If there are existing iterations, the additional iteration will be
   * combined in a weighted manner.
   *
   * @param kdt     a k-d tree containing the scene's geometry
   * @param lights  a vector containing all area lights in the scene
   * @param name    the name of the output EXR file
   */
  void renderOnce(
    const KDTree& kdt,
    const std::vector<Geom*>& lights,
    std::string name
  );

  /**
   * Renders multiple additional path-tracing iterations.
   * To render infinite iterations, specify iterations = -1.
   *
   * @param kdt        a k-d tree containing the scene's geometry
   * @param lights     a vector containing all area lights in the scene
   * @param name       the name of the output EXR file
   * @param iterations the number of iterations to render; if < 0, then this
   *                   function will run forever
   */
  void renderMultiple(
    const KDTree& kdt,
    const std::vector<Geom*>& lights,
    std::string name,
    int iterations
  );
};
