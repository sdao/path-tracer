#pragma once
#include "core.h"

/**
 * A material that specified how light interacts with geometry.
 */
class Material {
public:
  virtual ~Material();

  /**
   * Determines whether another ray should be cast as a consequence of a
   * lightray hitting a surface.
   *
   * @param incoming the incoming ray that struck the surface
   * @param isect    the intersection information for the incoming ray
   * @param rng      the per-thread RNG in use
   * @returns        a lightray to cast as a consequence; a zero-length ray will
   *                 terminate the path
   */
  virtual LightRay propagate(
    const LightRay& incoming,
    const Intersection& isect,
    Randomness& rng
  ) const = 0;
};
