#pragma once

#define EIGEN_NO_DEBUG 1

#include <OpenEXR/ImfRgbaFile.h>
#include <OpenEXR/ImfArray.h>
#include <cmath>
#include <limits>
#include <vector>
#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/Geometry>

typedef Eigen::Vector3f vec; /**< A 3D single-precision vector. */
typedef Eigen::Vector3d dvec; /**< A 3D double-precision vector. */

/** An enumeration of standard axes in 3D space. */
enum axis { X_AXIS = 0, Y_AXIS = 1, Z_AXIS = 2, INVALID_AXIS = -1 };

namespace math {

  /** A very small nonzero value. */
  static constexpr float VERY_SMALL = 0.0001f;

  /** Clamps a value between 0 and 1. */
  inline float clamp(float x) {
    return x < 0 ? 0 : (x > 1 ? 1 : x);
  }

  /**
   * Generates an orthonormal coordinate basis. The first vector must be given,
   * and the other two orthogonal vectors will be generated from it.
   * Taken from page 63 of Pharr & Humphreys' Physically-Based Rendering.
   *
   * @param v1 [in]  the first unit (normalized) vector of the basis
   * @param v2 [out] the second unit vector, generated from the first
   * @param v3 [out] the third unit vector, generated from the first
   */
  inline void coordSystem(const vec& v1, vec* v2, vec* v3) {
    if (fabsf(v1.x()) > fabsf(v1.y())) {
      float invLen = 1.0f / sqrtf(v1.x() * v1.x() + v1.z() * v1.z());
      *v2 = vec(-v1.z() * invLen, 0.0f, v1.x() * invLen);
    } else {
      float invLen = 1.0f / sqrtf(v1.y() * v1.y() + v1.z() * v1.z());
      *v2 = vec(0.0f, v1.z() * invLen, -v1.y() * invLen);
    }
    *v3 = v1.cross(*v2);
  }

  /**
   * Copies color data from a 2D vector into an OpenEXR array.
   *
   * @param w       the width of the vector (number of elements per row)
   * @param h       the height of the vector (number of row elements)
   * @param data    the vector to copy from; its elements are rows, whose
   *                elements in turn contain the actual color data
   * @param exrData the 2D OpenEXR array to which the data will be copied
   */
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
        rgba.r = float(p.x());
        rgba.g = float(p.y());
        rgba.b = float(p.z());
        rgba.a = 1.0f;
      }
    }
  }

  /**
   * Determines whether a number is zero, within a small epsilon.
   */
  inline bool isNearlyZero(float x) {
    return fabsf(x) < std::numeric_limits<float>::epsilon();
  }

  /**
   * Determines whether a vec's magnitude is zero, within a small epsilon.
   */
  inline bool isNearlyZero(const vec& v) {
    return isNearlyZero(v.squaredNorm());
  }

  /**
   * Determines whether a vec's components are each exactly 0.
   */
  inline bool isExactlyZero(const vec& v) {
    return v.x() == 0.0f && v.y() == 0.0f && v.z() == 0.0f;
  }

  /**
   * Determines whether a number is positive, within a small epsilon.
   */
  inline bool isPositive(float x) {
    return x > std::numeric_limits<float>::epsilon();
  }

  /**
   * Calculates the base-2 logarithm of a number.
   */
  inline float log2(float x) {
    static float invLog2 = 1.0f / logf(2.0f);
    return logf(x) * invLog2;
  }

  /**
   * Returns the axis enumeration value for a given int, or INVALID_AXIS if the
   * int can't be converted.
   */
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

  /**
   * Same as GLSL reflect.
   * See <https://www.opengl.org/sdk/docs/man4/html/reflect.xhtml>.
   *
   * @param I the incoming vector to reflect
   * @param N the normal at the surface over which to reflect
   * @returns the outgoing reflection vector
   */
  inline vec reflect(const vec& I, const vec& N) {
    return I - 2.0f * N.dot(I) * N;
  }

  /**
   * Same as GLSL refract.
   * See <https://www.opengl.org/sdk/docs/man4/html/refract.xhtml>.
   *
   * @param I   the incoming vector to refract
   * @param N   the normal at the surface to refract across;
   *            the normal points from the transmitting medium towards the
   *            incident medium
   * @param eta the ratio of the incoming IOR over the transmitting IOR
   * @returns   the outgoing refraction vector
   */
  inline vec refract(const vec& I, const vec& N, float eta) {
    float d = N.dot(I);
    float k = 1.0f - eta * eta * (1.0f - d * d);
    if (k < 0.0f) {
      return vec(0, 0, 0);
    } else {
      return (eta * I) - ((eta * d + sqrtf(k)) * N);
    }
  }

}
