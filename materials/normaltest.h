#pragma once
#include "../material.h"

namespace materials {

  class normaltest : public material {
  public:
    normaltest();
    
    virtual lightray propagate(
      const lightray& incoming,
      intersection& isect,
      randomness& rng
    ) const;
  };
  
}
