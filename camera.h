#pragma once
#include <tbb/tbb.h>
#include <vector>
#include "core.h"
#include "geom.h"
#include "kdtree.h"
#include "image.h"

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

  const Transform camToWorldXform;
  const Vec eyeWorldSpace;
  float farPlaneUp;
  float farPlaneRight;
  Vec farPlaneOrigin;
  
  Randomness masterRng; /**< The RNG used to seed the per-row RNGs. */
  std::vector<unsigned> rowSeeds; /**< The per-row RNG seeds. */

  Image img; /**< The rendered and filtered image. */

  int iters; /** The current number of path-tracing iterations done. */

  /**
   * Traces a path starting with the given ray, and returns the sampled
   * radiance.
   *
   * @param r          the ray that starts the path
   * @param kdt        a k-d tree containing the scene's geometry
   * @param lights     a vector containing all area lights in the scene
   */
  Vec trace(
    LightRay r,
    Randomness& rng,
    const KDTree& kdt,
    const std::vector<Geom*>& lights
  ) const;

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
   * @param kdt         a k-d tree containing the scene's geometry
   * @param lights      a vector containing all area lights in the scene
   */
  Vec uniformSampleOneLight(
    Randomness& rng,
    const LightRay& incoming,
    const Intersection& isect,
    const Material* mat,
    const KDTree& kdt,
    const std::vector<Geom*>& lights
  ) const;

public:
  /**
   * Constructs a camera.
   *
   * @param xform  the transformation from camera space to world space
   * @param ww     the width of the output image, in pixels
   * @param hh     the height of the output image, in pixels
   * @param fov    the field of view (horizontal or vertical, whichever is
   *               smaller), in radians
   */
  Camera(Transform xform, long ww, long hh, float fov = math::PI_4);

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
