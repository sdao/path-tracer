#pragma once
#include <OpenEXR/ImfRgbaFile.h>
#include <OpenEXR/ImfArray.h>
#include <cmath>
#include <limits>
#include <vector>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "randomness.h"

typedef glm::vec3 vec;
typedef glm::dvec3 dvec;

enum axis { X_AXIS = 0, Y_AXIS = 1, Z_AXIS = 2, INVALID_AXIS = -1 };

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

  inline float log2(float x) {
    static float invLog2 = 1.0f / logf(2.0f);
    return logf(x) * invLog2;
  }

  inline bool unsafeEquals(float x, float y) {
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wfloat-equal"
    return x == y;
    #pragma clang diagnostic pop
  }

  inline axis axisFromInt(int x) {
    switch (x) {
      case 0:
        return X_AXIS;
      case 1:
        return Y_AXIS;
      case 2:
        return Z_AXIS;
      default:
        return INVALID_AXIS;
    }
  }

}

struct ray {
  vec origin;
  vec direction;

  ray(vec o, vec d) : origin(o), direction(d) {}
  ray() : origin(0), direction(0) {}

  inline vec at(float d) const { return origin + direction * d; }

  friend std::ostream& operator<<(std::ostream& os, const ray& r) {
    os << "<origin: " << glm::to_string(r.origin)
       << ", direction: " << glm::to_string(r.direction) << ">";
    return os;
  }
};

struct lightray : public ray {
  vec color;

  lightray(vec o, vec d, vec c) : ray(o, d), color(c) {}
  lightray(vec o, vec d) : ray(o, d), color(1) {}
  lightray() : ray(), color(1) {}

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

struct bbox {
  vec min;
  vec max;

  bbox() : min(0), max(0) {}

  bbox(vec a, vec b) {
    min = vec(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z));
    max = vec(std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z));
  }

  inline void expand(const vec& v) {
    min = vec(std::min(min.x, v.x), std::min(min.y, v.y), std::min(min.z, v.z));
    max = vec(std::max(max.x, v.x), std::max(max.y, v.y), std::max(max.z, v.z));
  }

  inline void expand(float f = 0.1f) {
    min.x -= f;
    min.y -= f;
    min.z -= f;

    max.x += f;
    max.y += f;
    max.z += f;
  }

  inline void expand(const bbox& b) {
    min = vec(
      std::min(min.x, b.min.x),
      std::min(min.y, b.min.y),
      std::min(min.z, b.min.z)
    );
    max = vec(
      std::max(max.x, b.max.x),
      std::max(max.y, b.max.y),
      std::max(max.z, b.max.z)
    );
  }

  inline float surfaceArea() const {
    vec d = max - min;
    return 2.0f * (d.x * d.y + d.x * d.z + d.y * d.z);
  }

  inline axis maximumExtent() const {
    vec d = max - min;
    if (d.x > d.y && d.x > d.z) {
      return X_AXIS;
    } else if (d.y > d.z) {
      return Y_AXIS;
    } else {
      return Z_AXIS;
    }
  }

  inline bool intersect(const ray &r, float* t0_out, float* t1_out) const {
    float t0 = 0.0f;
    float t1 = std::numeric_limits<float>::max();
    for (int i = 0; i < 3; ++i) {
      // Update interval for `i`th bounding box slab.
      float invRayDir = 1.0f / r.direction[i];
      float tNear = (min[i] - r.origin[i]) * invRayDir;
      float tFar = (max[i] - r.origin[i]) * invRayDir;
      // Update parametric interval from slab intersection `t`s.
      if (tNear > tFar) std::swap(tNear, tFar);
      t0 = tNear > t0 ? tNear : t0;
      t1 = tFar < t1? tFar : t1;
      if (t0 > t1) return false;
    }
    if (t0_out) *t0_out = t0;
    if (t1_out) *t1_out = t1;
    return true;
  }

  friend std::ostream& operator<<(std::ostream& os, const bbox& b) {
    os << "<min: " << glm::to_string(b.min)
       << ", max: " << glm::to_string(b.max) << ">";
    return os;
  }
};

struct intersection {
  vec position;
  vec normal;
  vec tangent;
  vec cotangent;
  float distance;

  intersection()
    : position(0), normal(0), tangent(0), cotangent(0),
      distance(std::numeric_limits<float>::max()) {}
  intersection(vec p, vec n, float d)
    : position(p), normal(n), tangent(0), cotangent(0), distance(d) {}
  intersection(vec p, vec n, vec tang, vec cotang, float d)
    : position(p),
      normal(n),
      tangent(tang),
      cotangent(cotang),
      distance(d) {}

  inline bool hit() const {
    return distance < std::numeric_limits<float>::max();
  }

  /**
   * Generates a random ray in the hemisphere of the normal.
   * See <http://femto.cs.illinois.edu/faqs/cga-faq.html#S6.08>,
   * specifically question 6.08 of the comp.graphics.algorithms FAQ.
   *
   * If the tangent is exactly zero-length, we assume that it has not been
   * computed and we will compute it and the cotangent.
   * Otherwise, we trust that the tangent and cotangent are correct.
   */
  inline vec uniformSampleHemisphere(randomness& rng) {
    if (math::isExactlyZero(tangent)) {
      math::coordSystem(normal, &tangent, &cotangent);
    }

    float z = rng.nextFloat(1.0f);
    float t = rng.nextFloat(float(M_PI * 2.0));
    float r = sqrtf(1.0f - (z * z));
    float x = r * cosf(t);
    float y = r * sinf(t);

    return (z * normal) + (x * tangent) + (y * cotangent);
  }

  /**
   * Half-angle must be between 0 and Pi/2.
   *
   * Handy Mathematica code for checking that this works:
   * R[a_] := (h = Cos[Pi/2];
   *   z = RandomReal[{h, 1}];
   *   t = RandomReal[{0, 2*Pi}];
   *   r = Sqrt[1 - z^2];
   *   x = r*Cos[t];
   *   y = r*Sin[t];
   *   {x, y, z})
   *
   * ListPointPlot3D[Map[R, Range[1000]], BoxRatios -> Automatic]
   */
  inline vec uniformSampleCone(randomness& rng, float halfAngle) {
    if (math::isExactlyZero(tangent)) {
      math::coordSystem(normal, &tangent, &cotangent);
    }

    float h = cosf(halfAngle);
    float z = rng.nextFloat(h, 1.0f);
    float t = rng.nextFloat(float(M_PI * 2.0));
    float r = sqrtf(1.0f - (z * z));
    float x = r * cosf(t);
    float y = r * sinf(t);

    return (z * normal) + (x * tangent) + (y * cotangent);
  }
};
