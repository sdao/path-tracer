#pragma once
#include "../material.h"

namespace materials {

  /**
   * A test material that emits colored RGB light based on the XYZ normals.
   */
  class normaltest : public material {
  public:
    /**
     * Constructs a normaltest material.
     */
    normaltest();

    virtual lightray propagate(
      const lightray& incoming,
      const intersection& isect,
      randomness& rng
    ) const;
  };

}
