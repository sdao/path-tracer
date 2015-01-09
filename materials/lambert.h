#pragma once
#include "../material.h"

namespace materials {

  /**
   * A perfectly-ideal diffuse reflector using Lambertian reflectance.
   */
  class Lambert : public Material {
  protected:
    virtual Vec evalBSDFLocal(
      const Vec& incoming,
      const Vec& outgoing
    ) const override;
    
  public:
    const Vec albedo; /**< The albedo (diffuse color). */

    /** Constructs a diffuse material with the given albedo (diffuse color). */
    Lambert(Vec a);

    /** Constructs a diffuse material from the given parser. */
    Lambert(const Parser& p);

    virtual bool shouldDirectIlluminate() const override;
  };

}
