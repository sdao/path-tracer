#pragma once
#include "core.h"
#include "geom.h"
#include "material.h"
#include "kdtree.h"

class AreaLight {
public:
  const Vec color;

  AreaLight(Vec c);

  Vec directIlluminate(
    Randomness& rng,
    const Ray& incoming,
    const Intersection& isect,
    const Material* mat,
    const Geom* emissionObj,
    const KDTree& kdt
  ) const;
};
