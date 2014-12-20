#pragma once
#include "core.h"

/**
 * A material that specified how light interacts with geometry.
 */
class material {
public:
  virtual ~material();
  
  /**
   * Determines whether another ray should be cast as a consequence of a
   * lightray hitting a surface.
   *
   * @param incoming the incoming ray that struck the surface
   * @param isect    the intersection information for the incoming ray
   * @param rng      the per-thread RNG in use
   * @returns        a lightray to cast as a consequence; use a colored
   *                 zero-length ray to indicate that we have reached an emitter
   *                 and a black zero-length ray to indicate that the path gives
   *                 no light
   */
  virtual lightray propagate(
    const lightray& incoming,
    intersection& isect,
    randomness& rng
  ) const = 0;
};
