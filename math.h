#pragma once

#define EIGEN_NO_DEBUG 1

#include <OpenEXR/ImfRgbaFile.h>
#include <OpenEXR/ImfArray.h>
#include <cmath>
#include <limits>
#include <vector>
#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/Geometry>
#include "randomness.h"

using std::min;
using std::max;

typedef Eigen::Vector3f vec; /**< A 3D single-precision vector. */
typedef Eigen::Vector3d dvec; /**< A 3D double-precision vector. */

/** An enumeration of standard axes in 3D space. */
enum axis { X_AXIS = 0, Y_AXIS = 1, Z_AXIS = 2, INVALID_AXIS = -1 };

namespace math {

  /** A very small nonzero value. */
  static constexpr float VERY_SMALL = 0.0001f;

  /** Pi as a single-precision float. */
  static constexpr float PI = float(M_PI);

  /** 2 * Pi as a single-precision float. */
  static constexpr float TWO_PI = float(2.0 * M_PI);

  /** Pi / 2 as a single-precision float. */
  static constexpr float PI_2 = float(M_PI_2);

  /** Pi / 4 as a single-precision float. */
  static constexpr float PI_4 = float(M_PI_4);

  /** 1 / Pi as a single-precision float. */
  static constexpr float INV_PI = float(M_1_PI);

  /** Clamps a value x between a and b. */
  inline float clamp(float x, float a = 0.0f, float b = 1.0f) {
    return x < a ? a : (x > b ? b : x);
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
   * Converts a world-space vector to a local coordinate system.
   * The resulting coordinates are (x, y, z), where x is the weight of the
   * tangent, y is the weight of the binormal, and z is the weight of the
   * normal.
   */
  inline vec worldToLocal(
    const vec& world,
    const vec& tangent,
    const vec& binormal,
    const vec& normal
  ) {
    return vec(
      world.dot(tangent),
      world.dot(binormal),
      world.dot(normal)
    );
  }

  /**
   * Converts a local-space vector back to world-space. The local-space vector
   * should be (x, y, z), where x is the weight of the tangent, y is the weight
   * of the binormal, and z is the weight of the normal.
   */
  inline vec localToWorld(
    const vec& local,
    const vec& tangent,
    const vec& binormal,
    const vec& normal
  ) {
    return vec(
      tangent.x() * local.x() + binormal.x() * local.y()
        + normal.x() * local.z(),
      tangent.y() * local.x() + binormal.y() * local.y()
        + normal.y() * local.z(),
      tangent.z() * local.x() + binormal.z() * local.y()
        + normal.z() * local.z()
    );
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

  /**
   * Returns Cos[Theta] of a vector where Theta is the polar angle of the vector
   * in spherical coordinates.
   */
  inline float cosTheta(const vec& v) { return v.z(); }

  /**
   * Returns Abs[Cos[Theta]] of a vector where Theta is the polar angle of the
   * vector in spherical coordinates.
   */
  inline float absCosTheta(const vec& v) { return fabsf(v.z()); }

  /**
   * Returns Sin[Theta]^2 of a vector where Theta is the polar angle of the
   * vector in spherical coordinates.
   */
  inline float sinTheta2(const vec& v) {
    return max(0.0f, 1.0f - cosTheta(v) * cosTheta(v));
  }

  /**
   * Returns Sin[Theta] of a vector where Theta is the polar angle of the vector
   * in spherical coordinates.
   */
  inline float sinTheta(const vec& v) {
    return sqrtf(sinTheta2(v));
  }

  /**
   * Returns Cos[Phi] of a vector where Phi is the azimuthal angle of the vector
   * in spherical coordinates.
   */
  inline float cosPhi(const vec& v) {
    float sinT = sinTheta(v);
    if (sinT == 0.0f) {
      return 1.0f;
    }
    return clamp(v.x() / sinT, -1.0f, 1.0f);
  }

  /**
   * Returns Sin[Phi] of a vector where Phi is the azimuthal angle of the vector
   * in spherical coordinates.
   */
  inline float sinPhi(const vec& v) {
    float sinT = sinTheta(v);
    if (sinT == 0.0f) {
      return 0.0f;
    }
    return clamp(v.y() / sinT, -1.0f, 1.0f);
  }

  /**
   * Samples a unit disk, ensuring that the samples are uniformally distributed
   * throughout the area of the disk.
   *
   * Taken from Pharr & Humphreys' p. 667.
   *
   * @param rng      the per-thread RNG in use
   * @param dx [out] the x-coordinate of the sample
   * @param dy [out] the y-coordinate of the sample
   */
  inline void areaSampleDisk(randomness& rng, float* dx, float* dy) {
    float sx = rng.nextFloat(-1.0f, 1.0f);
    float sy = rng.nextFloat(-1.0f, 1.0f);

    // Handle degeneracy at the origin.
    if (sx == 0.0f && sy == 0.0f) {
      *dx = 0.0f;
      *dy = 0.0f;
      return;
    }

    float r;
    float theta;
    if (sx >= -sy) {
      if (sx > sy) {
        // Region 1.
        r = sx;
        if (sy > 0.0f) {
          theta = sy / r;
        } else {
          theta = 8.0f + sy / r;
        }
      } else {
        // Region 2.
        r = sy;
        theta = 2.0f - sx / r;
      }
    } else {
      if (sx <= sy) {
        // Region 3.
        r = -sx;
        theta = 4.0f - sy / r;
      } else {
        // Region 4.
        r = -sy;
        theta = 6.0f + sx / r;
      }
    }
    theta *= math::PI_4;
    *dx = r * cosf(theta);
    *dy = r * sinf(theta);
  }

  /**
   * Samples a unit hemisphere with a cosine-weighted distribution.
   * Directions with a higher cosine value (more parallel to the normal) are
   * more likely to be chosen than those with a lower cosine value (more
   * perpendicular to the normal).
   *
   * Taken from Pharr & Humphreys' p. 669.
   *
   * @param rng                  the per-thread RNG in use
   * @param directionOut   [out] a cosine-weighted random vector in the
   *                             hemisphere; the pointer must not be null
   * @param probabilityOut [out] the probability of the sampled direction; the
   *                             pointer must not be null
   */
  inline void cosineSampleHemisphere(
    randomness& rng,
    vec* directionOut,
    float* probabilityOut
  ) {
    vec ret;
    areaSampleDisk(rng, &ret[0], &ret[1]);
    ret[2] = sqrtf(max(0.0f, 1.0f - ret[0] * ret[0] - ret[1] * ret[1]));
    *directionOut = ret;
    *probabilityOut = absCosTheta(ret) * INV_PI;
  }

}
