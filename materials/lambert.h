#pragma once
#include "../material.h"

namespace materials {

  /**
   * A perfectly-ideal diffuse reflector using Lambertian reflectance.
   */
  class Lambert : public Material {
  public:
    const Vec albedo; /**< The albedo (diffuse color). */

    /** Constructs a diffuse material with the given albedo (diffuse color). */
    Lambert(Vec a);

    virtual Vec evalBSDFLocal(
      const Vec& incoming,
      const Vec& outgoing
    ) const override;

    virtual bool shouldDirectIlluminate() const override;
  };

}
