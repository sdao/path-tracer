#pragma once
#include "../material.h"

namespace materials {

  class emitter : public material {
  public:
    const vec color;

    emitter(vec c);

    virtual lightray propagate(
      const lightray& incoming,
      intersection& isect,
      randomness& rng
    ) const;
  };

}
