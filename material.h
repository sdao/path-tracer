#pragma once
#include <memory>
#include <random>
#include <limits>
#include "math.h"

#define IOR_VACUUM 1.0f

class material {
protected:
  material(vec d) : debug_color(d) {}

public:
  vec debug_color;
  virtual ~material() {}
  virtual lightray propagate(const lightray& incoming,
    const intersection& isect,
    std::mt19937& rng) = 0;
};

typedef std::shared_ptr<material> materialptr;

class idealemitter : public material {
public:
  vec color;

  idealemitter(vec c) : color(c), material(c) {}

  virtual lightray propagate(const lightray& incoming,
    const intersection& isect,
    std::mt19937& rng) {
    return lightray(vec(0), vec(0), incoming.color * color);
  }
};

class idealspecular : public material {
public:
  idealspecular() : material(vec(1)) {}

  virtual lightray propagate(const lightray& incoming,
    const intersection& isect,
    std::mt19937& rng) {
    vec reflectVector = glm::reflect(incoming.unit().direction, isect.normal);
    return lightray(isect.position + reflectVector * 0.01f,
      reflectVector,
      incoming.color);
  }
};

class idealdiffuse : public material {
  std::normal_distribution<float> dist;

public:
  vec color;

  idealdiffuse(vec c) : color(c), dist(), material(c) {}

  virtual lightray propagate(const lightray& incoming,
    const intersection& isect,
    std::mt19937& rng) {
    // Generate random ray in hemisphere of normal.
    // See <http://mathworld.wolfram.com/SpherePointPicking.html>
    float x1 = dist(rng);
    float x2 = dist(rng);
    float x3 = dist(rng);
    float denom = 1.0f / sqrt(x1 * x1 + x2 * x2 + x3 * x3);
    float y1 = fabsf(x1 * denom);
    float y2 = x2 * denom;
    float y3 = x3 * denom;

    vec v1 = isect.normal;
    vec v2;
    vec v3;
    math::coordSystem(v1, &v2, &v3);
    vec reflectVector = (v1 * y1) + (v2 * y2) + (v3 * y3);

    return lightray(isect.position + reflectVector * 0.01f,
      reflectVector,
      incoming.color * color);
  }
};

class glossyspecular : public material {
  std::normal_distribution<float> dist;

public:
  float glossiness;

  glossyspecular(float g) : glossiness(g), dist(), material(vec(1)) {}

  virtual lightray propagate(const lightray& incoming,
  const intersection& isect,
  std::mt19937& rng) {
    // Generate random ray in hemisphere of normal.
    // See <http://mathworld.wolfram.com/SpherePointPicking.html>
    float x1 = dist(rng);
    float x2 = dist(rng);
    float x3 = dist(rng);
    float denom = 1.0f / sqrt(x1 * x1 + x2 * x2 + x3 * x3);
    float y1 = fabsf(x1 * denom);
    float y2 = x2 * denom;
    float y3 = x3 * denom;

    vec v1 = isect.normal;
    vec v2;
    vec v3;
    math::coordSystem(v1, &v2, &v3);
    vec reflectVector = (v1 * y1) + (v2 * y2) + (v3 * y3);

    return lightray(isect.position + reflectVector * 0.01f,
    glossiness * reflectVector + (1.0f - glossiness) * isect.normal,
    incoming.color);
  }
};

class fresnelrefract : public material {
  float idxRefract;
  std::uniform_real_distribution<float> dist;

public:
  fresnelrefract(float ior = 1.5f) : material(vec(1)), idxRefract(ior) {}

  virtual lightray propagate(const lightray& incoming,
  const intersection& isect,
  std::mt19937& rng) {
    // Entering = are normal and ray in opposite directions?
    bool entering = glm::dot(isect.normal, incoming.direction) < 0;
    vec alignedNormal;
    float nIncident, nTransmit; // Indices of refraction.
    if (entering) {
      // Note: geometry will return surface normal pointing outwards.
      // If we are entering, this is the right normal.
      // If we are exiting, since geometry is single-shelled, we will need
      // to flip the normal.
      alignedNormal = isect.normal;
      nIncident = IOR_VACUUM;
      nTransmit = idxRefract;
    } else {
      alignedNormal = -isect.normal;
      nIncident = idxRefract;
      nTransmit = IOR_VACUUM;
    }

    // Calculate reflection vector using GLM.
    vec reflectVector = glm::reflect(incoming.unit().direction, alignedNormal);

    // Calculate refraction vector using GLM.
    vec refractVector = glm::refract(incoming.unit().direction,
      alignedNormal, nIncident / nTransmit);
    if (math::isNearlyZero(glm::length(refractVector))) {
      // Total internal reflection. Must reflect.
      return lightray(isect.position + reflectVector * 0.01f,
        reflectVector,
        incoming.color);
    }

    // Choose whether to reflect or refract using Fresnel equations.
    // Uses Shlick's approximation, see <http://graphics.stanford.edu/
    // courses/cs148-10-summer/docs/2006--degreve--reflection_refraction.pdf>.
    float r0_temp = (nIncident - nTransmit) / (nIncident + nTransmit);
    float r0 = r0_temp * r0_temp;
    float cos_temp;
    if (nIncident < nTransmit) {
      // Equivalent to condition: entering == true
      // (e.g. nI = 1 (air), nT = 1.5 (glass))
      // Theta = angle of incidence.
      cos_temp = 1.0f - glm::dot(-incoming.unit().direction, alignedNormal);
    } else {
      // Equivalent to condition: entering == false
      // Theta = angle of refraction.
      cos_temp = 1.0f - glm::dot(refractVector, -alignedNormal);
    }
    float refl = r0
      + (1.0f - r0) * cos_temp * cos_temp * cos_temp * cos_temp * cos_temp;
    float refr = 1.0f - refl;

    // Importance sampling probabilities.
    float probRefl = 0.25f + 0.5f * refl; // [0.25, 0.75] based on reflectance.
    float probRefr = 1.0f - probRefl;

    // Probabilistically choose to refract or reflect.
    if (dist(rng) < probRefl) {
      // Higher reflectance = higher probability of reflecting.
      return lightray(isect.position + reflectVector * 0.01f,
        reflectVector,
        incoming.color * refl / probRefl); // Adjust color for probabilities.
    } else {
      return lightray(isect.position + refractVector * 0.01f,
        refractVector,
        incoming.color * refr / probRefr); // Adjust color for probabilities.
    }
  }

};
