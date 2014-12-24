#pragma once
#include "../material.h"

namespace materials {

  /**
   * A perfectly-ideal diffuse reflector.
   */
  class lambert : public material {
  protected:
    virtual vec evalBRDF(
      const vec& incoming,
      const vec& outgoing
    ) const override;

  public:
    const vec albedo; /**< The albedo (diffuse color). */

    /** Constructs a diffuse material with the given albedo (diffuse color). */
    lambert(vec a);
  };

}
