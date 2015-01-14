#pragma once

#define EIGEN_NO_DEBUG

#include <cmath>
#include <limits>
#include <vector>
#include <Eigen/Dense>
#include <Eigen/Geometry>
#include "randomness.h"

using std::min;
using std::max;

typedef Eigen::Vector3f Vec; /**< A 3D single-precision vector. */
typedef Eigen::Vector2f Vec2; /**< A 2D single-precision vector. */
typedef Eigen::Vector4f Vec4; /**< A 4D single-precision vector. */
typedef Eigen::Affine3f Transform; /**< An affine transformation in 3D space. */

/** An enumeration of standard axes in 3D space. */
enum axis { X_AXIS = 0, Y_AXIS = 1, Z_AXIS = 2, INVALID_AXIS = -1 };

namespace math {

  /** A very small nonzero value. */
  static constexpr float VERY_SMALL = 0.0001f;

  /** A very big non-infinite value. */
  static constexpr float VERY_BIG = std::numeric_limits<float>::max();

  /** Pi as a single-precision float. */
  static constexpr float PI = float(M_PI);

  /** 2 * Pi as a single-precision float. */
  static constexpr float TWO_PI = float(2.0 * M_PI);

  /** 4 * Pi as a single-precision float. */
  static constexpr float FOUR_PI = float(4.0 * M_PI);

  /** Pi / 2 as a single-precision float. */
  static constexpr float PI_2 = float(M_PI_2);

  /** Pi / 3 as a single-precision float. */
  static constexpr float PI_3 = float(M_PI / 3.0);

  /** Pi / 4 as a single-precision float. */
  static constexpr float PI_4 = float(M_PI_4);

  /** Pi / 6 as a single-precision float. */
  static constexpr float PI_6 = float(M_PI / 6.0);

  /** 1 / Pi as a single-precision float. */
  static constexpr float INV_PI = float(M_1_PI);

  /** Sqrt[3] as a single-precision float. */
  static constexpr float SQRT_3 = 1.7320508075688772935274463415f;

  /** The number of steradians in a sphere (4 * Pi). */
  static constexpr float STERADIANS_PER_SPHERE = FOUR_PI;

  /** Clamps a value x between a and b. */
  inline float clamp(float x, float a = 0.0f, float b = 1.0f) {
    return x < a ? a : (x > b ? b : x);
  }

  /** Clamps a value x between a and b. */
  template< class T >
  inline T clampAny(T x, T a, T b) {
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
  inline void coordSystem(const Vec& v1, Vec* v2, Vec* v3) {
    if (fabsf(v1.x()) > fabsf(v1.y())) {
      float invLen = 1.0f / sqrtf(v1.x() * v1.x() + v1.z() * v1.z());
      *v2 = Vec(-v1.z() * invLen, 0.0f, v1.x() * invLen);
    } else {
      float invLen = 1.0f / sqrtf(v1.y() * v1.y() + v1.z() * v1.z());
      *v2 = Vec(0.0f, v1.z() * invLen, -v1.y() * invLen);
    }
    *v3 = v1.cross(*v2);
  }

  /**
   * Converts a world-space vector to a local coordinate system.
   * The resulting coordinates are (x, y, z), where x is the weight of the
   * tangent, y is the weight of the binormal, and z is the weight of the
   * normal.
   */
  inline Vec worldToLocal(
    const Vec& world,
    const Vec& tangent,
    const Vec& binormal,
    const Vec& normal
  ) {
    return Vec(
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
  inline Vec localToWorld(
    const Vec& local,
    const Vec& tangent,
    const Vec& binormal,
    const Vec& normal
  ) {
    return Vec(
      tangent.x() * local.x() + binormal.x() * local.y()
        + normal.x() * local.z(),
      tangent.y() * local.x() + binormal.y() * local.y()
        + normal.y() * local.z(),
      tangent.z() * local.x() + binormal.z() * local.y()
        + normal.z() * local.z()
    );
  }

  /**
   * Evaluates a triangle filter with width = 0.5 (support = 1.0) for a
   * specified offset from the pixel center. The values are not normalized,
   * i.e., the integral of the filter over the 1x1 square around the point.
   * Thus, you should only use the filter weights relative to other weights.
   * 
   * In fact, Mathematica says that:
   * @code
   * In := Integrate[(0.5-Abs[x])*(0.5-Abs[y]), {x, -0.5, 0.5}, {y, -0.5, 0.5}]
   * Out = 0.0625
   * @endcode
   *
   * @param x     the x-offset from the pixel center, -width <= x <= width
   * @param y     the y-offset from the pixel center, -width <= x <= width
   * @param width the maximum x- or y- offset sampled from the pixel center
   * @returns the value of the filter
   */
  inline float triangleFilter(float x, float y, float width = 2.0f) {
    return max(0.0f, width - fabsf(x)) * max(0.0f, width - fabsf(y));
  }

  /**
   * Computes the 1-dimensional Mitchell filter with B = 1/3 and C = 1/3 for a
   * scaled offset from the pixel center. The values are not normalized.
   *
   * Pharr and Humphreys suggest on p. 398 of PBR that values of B and C should
   * be chosen such that B + 2C = 1.
   * GPU Gems <http://http.developer.nvidia.com/GPUGems/gpugems_ch24.html>
   * suggests the above values of B = 1/3 and C = 1/3.
   *
   * @param x the scaled x-offset from the pixel center, -1 <= x <= 1
   */
  inline float mitchellFilter(float x) {
    const float B = 1.0f / 3.0f;
    const float C = 1.0f / 3.0f;

    x = fabsf(2.0f * x); // Convert to the range [0, 2].

    if (x > 1.0f) {
      return ((-B - 6.0f * C) * (x * x * x)
        + (6.0f * B + 30.0f * C) * (x * x)
        + (-12.0f * B - 48.0f * C) * x
        + (8.0f * B + 24.0f * C)) * (1.0f / 6.0f);
    } else {
      return ((12.0f - 9.0f * B - 6.0f * C) * (x * x * x)
        + (-18.0f + 12.0f * B + 6.0f * C) * (x * x)
        + (6.0f - 2.0f * B)) * (1.0f / 6.0f);
    }
  }

  /**
   * Evaluates a 2-dimensional Mitchell filter at a specified offset from the
   * pixel center by separating and computing the 1-dimensional Mitchell
   * filter for the x- and y- offsets.
   *
   * @param x     the x-offset from the pixel center, -width <= x <= width
   * @param y     the y-offset from the pixel center, -width <= x <= width
   * @param width the maximum x- or y- offset sampled from the pixel center
   * @returns the value of the filter
   */
  inline float mitchellFilter(float x, float y, float width = 2.0f) {
    return mitchellFilter(x / width) * mitchellFilter(y / width);
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
  inline bool isNearlyZero(const Vec& v) {
    return isNearlyZero(v.squaredNorm());
  }

  /**
   * Determines whether a number is positive, within a small epsilon.
   */
  inline bool isPositive(float x) {
    return x > std::numeric_limits<float>::epsilon();
  }

  /**
   * Convenience function for determining whether all of a vec's components are
   * zero.
   */
  inline bool isVectorExactlyZero(const Vec& v) {
    return v.x() == 0.0f && v.y() == 0.0f && v.z() == 0.0f;
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
  inline Vec reflect(const Vec& I, const Vec& N) {
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
  inline Vec refract(const Vec& I, const Vec& N, float eta) {
    float d = N.dot(I);
    float k = 1.0f - eta * eta * (1.0f - d * d);
    if (k < 0.0f) {
      return Vec(0, 0, 0);
    } else {
      return (eta * I) - ((eta * d + sqrtf(k)) * N);
    }
  }

  /**
   * Returns Cos[Theta] of a vector where Theta is the polar angle of the vector
   * in spherical coordinates.
   */
  inline float cosTheta(const Vec& v) { return v.z(); }

  /**
   * Returns Abs[Cos[Theta]] of a vector where Theta is the polar angle of the
   * vector in spherical coordinates.
   */
  inline float absCosTheta(const Vec& v) { return fabsf(v.z()); }

  /**
   * Returns Sin[Theta]^2 of a vector where Theta is the polar angle of the
   * vector in spherical coordinates.
   */
  inline float sinTheta2(const Vec& v) {
    return max(0.0f, 1.0f - cosTheta(v) * cosTheta(v));
  }

  /**
   * Returns Sin[Theta] of a vector where Theta is the polar angle of the vector
   * in spherical coordinates.
   */
  inline float sinTheta(const Vec& v) {
    return sqrtf(sinTheta2(v));
  }

  /**
   * Returns Cos[Phi] of a vector where Phi is the azimuthal angle of the vector
   * in spherical coordinates.
   */
  inline float cosPhi(const Vec& v) {
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
  inline float sinPhi(const Vec& v) {
    float sinT = sinTheta(v);
    if (sinT == 0.0f) {
      return 0.0f;
    }
    return clamp(v.y() / sinT, -1.0f, 1.0f);
  }

  /**
   * Determines if two vectors in the same local coordinate space are in the 
   * same hemisphere.
   */
  inline bool localSameHemisphere(const Vec& u, const Vec& v) {
    return u.z() * v.z() >= 0.0f;
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
  inline void areaSampleDisk(Randomness& rng, float* dx, float* dy) {
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
   * Returns the probability that the given direction was sampled from a unit
   * hemisphere using a cosine-weighted distribution. (It does not matter
   * whether the hemisphere is on the positive or negative Z-axis.)
   */
  inline float cosineSampleHemispherePDF(const Vec& direction) {
    return absCosTheta(direction) * INV_PI;
  }

  /**
   * Samples a unit hemisphere with a cosine-weighted distribution.
   * Directions with a higher cosine value (more parallel to the normal) are
   * more likely to be chosen than those with a lower cosine value (more
   * perpendicular to the normal).
   *
   * Taken from Pharr & Humphreys p. 669.
   *
   * @param rng     the per-thread RNG in use
   * @param flipped whether to sample from the hemisphere on the negative
   *                Z-axis instead; false will sample from the positive
   *                hemisphere and true will sample from the negative hemisphere
   * @returns       a cosine-weighted random vector in the hemisphere;
   *                the pointer must not be null
   */
  inline Vec cosineSampleHemisphere(Randomness& rng, bool flipped) {
    Vec ret;
    areaSampleDisk(rng, &ret[0], &ret[1]);
    ret[2] = sqrtf(max(0.0f, 1.0f - ret[0] * ret[0] - ret[1] * ret[1]));
    if (flipped) {
      ret[2] *= -1.0f;
    }
    return ret;
  }

  /**
   * Returns the probability that any solid angle was sampled uniformly
   * from a unit sphere.
   */
  inline float uniformSampleSpherePDF() {
    return 1.0f / STERADIANS_PER_SPHERE;
  }

  /**
   * Uniformly samples from a unit sphere, with respect to the sphere's
   * surface area.
   *
   * @param rng the per-thread RNG in use
   * @returns   the uniformly-distributed random vector in the sphere
   */
  inline Vec uniformSampleSphere(Randomness& rng) {
    // See MathWorld <http://mathworld.wolfram.com/SpherePointPicking.html>.
    float x = rng.nextNormalFloat();
    float y = rng.nextNormalFloat();
    float z = rng.nextNormalFloat();
    float a = 1.0f / sqrtf(x * x + y * y + z * z);

    return Vec(a * x, a * y, a * z);
  }

  /**
   * Returns the probability that any solid angle already inside the given
   * cone was sampled uniformly from the cone. The cone is defined by the
   * half-angle of the subtended (apex) angle.
   *
   * @param halfAngle the half-angle of the cone
   * @returns         the probability that the angle was sampled
   */
  inline float uniformSampleConePDF(float halfAngle) {
    const float solidAngle = math::TWO_PI * (1.0f - cosf(halfAngle));
    return 1.0f / solidAngle;
  }

  /**
   * Returns the proabability that the given solid angle was sampled
   * uniformly from the given cone. The cone is defined by the half-angle of
   * the subtended (apex) angle. The probability is uniform if the direction
   * is actually in the cone, and zero if it is outside the cone.
   *
   * @param halfAngle the half-angle of the cone
   * @param direction the direction of the sampled vector
   * @returns         the probability that the angle was sampled
   */
  inline float uniformSampleConePDF(float halfAngle, const Vec& direction) {
    const float cosHalfAngle = cosf(halfAngle);
    const float solidAngle = math::TWO_PI * (1.0f - cosHalfAngle);
    if (cosTheta(direction) > cosHalfAngle) {
      // Within the sampling cone.
      return 1.0f / solidAngle;
    } else {
      // Outside the sampling cone.
      return 0.0f;
    }
  }

  /**
   * Generates a random ray in a cone around the positive z-axis, uniformly
   * with respect to solid angle.
   *
   * Handy Mathematica code for checking that this works:
   * \code
   * R[a_] := (h = Cos[Pi/2];
   *   z = RandomReal[{h, 1}];
   *   t = RandomReal[{0, 2*Pi}];
   *   r = Sqrt[1 - z^2];
   *   x = r*Cos[t];
   *   y = r*Sin[t];
   *   {x, y, z})
   *
   * ListPointPlot3D[Map[R, Range[1000]], BoxRatios -> Automatic]
   * \endcode
   *
   * @param rng       the per-thread RNG in use
   * @param halfAngle the half-angle of the cone's opening; must be between 0
   *                  and Pi/2 and in radians
   * @returns         a uniformally-random vector within halfAngle radians of
   *                  the positive z-axis
   */
  inline Vec uniformSampleCone(Randomness& rng, float halfAngle) {
    float h = cosf(halfAngle);
    float z = rng.nextFloat(h, 1.0f);
    float t = rng.nextFloat(float(PI * 2.0));
    float r = sqrtf(1.0f - (z * z));
    float x = r * cosf(t);
    float y = r * sinf(t);

    return Vec(x, y, z);
  }

  /**
   * Linearly interpolates between x and y. Where a = 0, x is returned, and
   * where a = 1, y is returned. If a < 0 or a > 1, this function will
   * extrapolate.
   */
  inline float lerp(float x, float y, float a) {
    return x + a * (y - x);
  }

  /**
   * Linearly interpolates between x and y. Where a <= 0, x is returned, and
   * where a >= 1, y is returned. No extrapolation will occur.
   */
  inline float clampedLerp(float x, float y, float a) {
    return lerp(x, y, clamp(a));
  }

  /**
   * Calculates the power heuristic for multiple importance sampling of
   * two separate functions.
   *
   * See Pharr & Humphreys p. 693 for more information.
   *
   * @param nf   number of samples taken with a Pf distribution
   * @param fPdf probability according to the Pf distribution
   * @param ng   number of samples taken with a Pg distribution
   * @param gPdf probability according to the Pg distribution
   * @returns    the weight according to the power heuristic
   */
  inline float powerHeuristic(int nf, float fPdf, int ng, float gPdf) {
    float f = nf * fPdf;
    float g = ng * gPdf;

    return (f * f) / (f * f + g * g);
  }

  inline Transform translation(Vec v) {
    return Transform(Eigen::Translation<float, 3>(v.x(), v.y(), v.z()));
  }

  inline Transform rotation(float angle, Vec axis) {
    return Transform(Eigen::AngleAxis<float>(angle, axis));
  }

  inline Transform rotationThenTranslation(float angle, Vec axis, Vec offset) {
    return translation(offset) * rotation(angle, axis);
  }

}
