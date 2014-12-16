#pragma once
#include <OpenEXR/ImfRgbaFile.h>
#include <OpenEXR/ImfArray.h>
#include <cmath>
#include <limits>
#include <vector>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "randomness.h"

typedef glm::vec3 vec;
typedef glm::dvec3 dvec;

namespace math {

  inline float clamp(float x) {
    return x < 0 ? 0 : (x > 1 ? 1 : x);
  }

  /**
   * Taken from page 63 of Pharr & Humphreys' Physically-Based Rendering.
   */
  inline void coordSystem(const vec& v1, vec* v2, vec* v3) {
    if (fabsf(v1.x) > fabsf(v1.y)) {
      float invLen = 1.0f / sqrtf(v1.x * v1.x + v1.z * v1.z);
      *v2 = vec(-v1.z * invLen, 0.0f, v1.x * invLen);
    } else {
      float invLen = 1.0f / sqrtf(v1.y * v1.y + v1.z * v1.z);
      *v2 = vec(0.0f, v1.z * invLen, -v1.y * invLen);
    }
    *v3 = glm::cross(v1, *v2);
  }

  inline void copyData(
    size_t w,
    size_t h,
    const std::vector< std::vector<dvec> >& data,
    Imf::Array2D<Imf::Rgba>& exrData
  ) {
    for (size_t y = 0; y < h; ++y) {
      for (size_t x = 0; x < w; ++x) {
        Imf::Rgba& rgba = exrData[long(y)][long(x)];
        const dvec &p = data[y][x];
        rgba.r = float(p.x);
        rgba.g = float(p.y);
        rgba.b = float(p.z);
        rgba.a = 1.0f;
      }
    }
  }

  inline bool isNearlyZero(float x) {
    return fabsf(x) < std::numeric_limits<float>::epsilon();
  }

  inline bool isNearlyZero(const vec& v) {
    return isNearlyZero(glm::length2(v));
  }

  inline bool isExactlyZero(const vec& v) {
    return v.x == 0.0f && v.y == 0.0f && v.z == 0.0f;
  }

  inline bool isPositive(float x) {
    return x > std::numeric_limits<float>::epsilon();
  }

}

struct ray {
  vec origin;
  vec direction;

  ray(vec o, vec d) : origin(o), direction(d) {}
  ray() : origin(0.0), direction(0.0) {}

  inline ray unit() const { return ray(origin, glm::normalize(direction)); }
  inline vec at(float d) const { return origin + direction * d; }
};

struct lightray : public ray {
  vec color;

  lightray(vec o, vec d, vec c) : ray(o, d), color(c) {}
  lightray(vec o, vec d) : ray(o, d), color(1) {}
  lightray() : ray(vec(0), vec(0)), color(1) {}

  inline bool isBlack() const {
    return math::isNearlyZero(color);
  }

  inline float energy() const {
    return std::max(std::max(color.x, color.y), color.z);
  }

  inline bool isZeroLength() const {
    return math::isNearlyZero(direction);
  }

  inline void kill() {
    origin = vec(0);
    direction = vec(0);
    color = vec(0);
  }
};

struct intersection {
  vec position;
  vec normal;
  vec tangent;
  vec cotangent;
  float distance;

  intersection()
    : position(0), normal(0), tangent(0), cotangent(0), distance(0.0f) {}
  intersection(vec p, vec n, float d)
    : position(p), normal(n), tangent(0), cotangent(0), distance(d) {}
  intersection(vec p, vec n, vec tang, vec cotang, float d)
    : position(p),
      normal(n),
      tangent(tang),
      cotangent(cotang),
      distance(d) {}

  inline bool hit() const { return distance > 0.0f; }

  /**
   * Generates a random ray in the hemisphere of normal by using a Gaussian
   * distribution.
   * See <http://mathworld.wolfram.com/SpherePointPicking.html>.
   *
   * If the tangent is exactly zero-length, we assume that it has not been
   * computed and we will compute it and the cotangent.
   * Otherwise, we trust that the tangent and cotangent are correct.
   */
  inline vec uniformSampleHemisphere(randomness& rng) {
    if (math::isExactlyZero(tangent)) {
      math::coordSystem(normal, &tangent, &cotangent);
    }

    float x1 = rng.nextNormalFloat();
    float x2 = rng.nextNormalFloat();
    float x3 = rng.nextNormalFloat();

    float denom = 1.0f / sqrtf(x1 * x1 + x2 * x2 + x3 * x3);
    float y1 = fabsf(x1 * denom);
    float y2 = x2 * denom;
    float y3 = x3 * denom;

    return (y1 * normal) + (y2 * tangent) + (y3 * cotangent);
  }
};
