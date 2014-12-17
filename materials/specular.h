#pragma once
#include "../material.h"

namespace materials {

  class specular : public material {
  public:
    specular();

    virtual lightray propagate(
      const lightray& incoming,
      intersection& isect,
      randomness& rng
    ) const;

    static material* make();
  };

}
