#pragma once
#include <memory>
#include <random>
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

class idealrefract : public material {
  float idxRefract;

public:
  idealrefract(float ior = 1.5f) : material(vec(1)), idxRefract(ior) {}

  virtual lightray propagate(const lightray& incoming,
  const intersection& isect,
  std::mt19937& rng) {
    vec reflectVector = glm::reflect(incoming.unit().direction, isect.normal);
    // Entering = are normal and ray in opposite directions?
    bool entering = glm::dot(isect.normal, incoming.direction) < 0;
    float snellsRatio =
      entering ? IOR_VACUUM / idxRefract : idxRefract / IOR_VACUUM;

    // If angle of incidence with normal is < critical angle, will reflect.
    
  }
};
