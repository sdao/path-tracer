#pragma once
#include "../material.h"

namespace materials {

  class glossy : public material {
  public:
    const float glossiness;

    glossy(float g);

    virtual lightray propagate(
      const lightray& incoming,
      intersection& isect,
      randomness& rng
    ) const;
  };

}
