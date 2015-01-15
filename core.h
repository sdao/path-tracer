#pragma once
#include <iostream>
#include <boost/format.hpp>
#include "math.h"
#include "randomness.h"

using std::min;
using std::max;

/**
 * A directed line segment.
 */
struct Ray {
  Vec origin; /**< The point at which the ray starts. */
  Vec direction; /**< The direction and magnitude of the ray. */

  /**
   * Constructs a ray with the given origin and direction.
   *
   * @param o the ray's origin
   * @param d the ray's direction (and magnitude, if desired)
   */
  Ray(const Vec& o, const Vec& d) : origin(o), direction(d) {}

  /**
   * Constructs a ray with origin at (0, 0, 0) and neither direction nor
   * magnitude.
   */
  Ray() : origin(0, 0, 0), direction(0, 0, 0) {}

  /**
   * Returns an interpolated point on the ray, parameterized by a number d.
   * Where d = 0, the origin is returned and where d = 1, (origin + direction)
   * is returned. This linear interpolation also works for d < 0 and d > 1.
   *
   * @param d the parameter at which to return a point
   * @returns the point on the ray given by the parameter
   */
  inline Vec at(float d) const { return origin + direction * d; }

  friend std::ostream& operator<<(std::ostream& os, const Ray& r) {
    os << boost::format("<origin: %1%, direction: %2%>")
      % r.origin % r.direction;
    return os;
  }
};

/**
 * A ray of light, i.e. a ray with an additional color component.
 */
struct LightRay : public Ray {
  Vec color; /**< The light color of the ray. */

  /**
   * Constructs a lightray with the given origin, direction, and color.
   *
   * @param o the ray's origin
   * @param d the ray's direction (and magnitude, if desired)
   * @param c the ray's light color
   */
  LightRay(const Vec& o, const Vec& d, const Vec& c) : Ray(o, d), color(c) {}

  /**
   * Constructs a white lightray with the given origin and direction.
   *
   * @param o the ray's origin
   * @param d the ray's direction (and magnitude, if desired)
   */
  LightRay(const Vec& o, const Vec& d) : Ray(o, d), color(1, 1, 1) {}

  /**
   * Constructs a white lightray with origin at (0, 0, 0) and neither direction
   * nor magnitude.
   */
  LightRay() : Ray(), color(1, 1, 1) {}

  /**
   * Determines whether the ray's color is black, within a small epsilon.
   */
  inline bool isBlack() const {
    return math::isNearlyZero(color);
  }

  /**
   * Returns the perceived luminance of the light's color, assuming it is RGB.
   */
  inline float luminance() const {
    return 0.21f * color.x() + 0.71f * color.y() + 0.08f * color.z();
  }

  /**
   * Determines whether the ray's magnitude is zero, with a small epsilon.
   */
  inline bool isZeroLength() const {
    return math::isNearlyZero(direction);
  }
};

/**
 * An axis-aligned bounding box.
 */
struct BBox {
  Vec lower; /**< The lower X, Y, and Z-axis bounds. */
  Vec upper; /**< The upper X, Y, and Z-axis bounds. */

  /** Constructs an empty bbox. */
  BBox() : lower(0, 0, 0), upper(0, 0, 0) {}

  /** Constructs a bbox containing the two given points. */
  BBox(const Vec& a, const Vec& b) {
    lower = Vec(min(a.x(), b.x()), min(a.y(), b.y()), min(a.z(), b.z()));
    upper = Vec(max(a.x(), b.x()), max(a.y(), b.y()), max(a.z(), b.z()));
  }

  /** Expands the bbox to also contain the given point. */
  inline void expand(const Vec& v) {
    lower = Vec(
      min(lower.x(), v.x()),
      min(lower.y(), v.y()),
      min(lower.z(), v.z())
    );
    upper = Vec(
      max(upper.x(), v.x()),
      max(upper.y(), v.y()),
      max(upper.z(), v.z())
    );
  }

  /**
   * Expands the dimensions of the bbox by a given amount along all six faces
   * of the box. The result is that the bbox is expanded by twice the given
   * amount along each axis.
   */
  inline void expand(float f = math::VERY_SMALL) {
    lower.x() -= f;
    lower.y() -= f;
    lower.z() -= f;

    upper.x() += f;
    upper.y() += f;
    upper.z() += f;
  }

  /**
   * Expands the bbox to also contain another given bbox.
   */
  inline void expand(const BBox& b) {
    lower = Vec(
      min(lower.x(), b.lower.x()),
      min(lower.y(), b.lower.y()),
      min(lower.z(), b.lower.z())
    );
    upper = Vec(
      max(upper.x(), b.upper.x()),
      max(upper.y(), b.upper.y()),
      max(upper.z(), b.upper.z())
    );
  }

  /**
   * Returns the surface area of the bbox.
   */
  inline float surfaceArea() const {
    Vec d = upper - lower;
    return 2.0f * (d.x() * d.y() + d.x() * d.z() + d.y() * d.z());
  }

  /**
   * Return the longest axis of the bbox.
   */
  inline axis maximumExtent() const {
    Vec d = upper - lower;
    if (d.x() > d.y() && d.x() > d.z()) {
      return X_AXIS;
    } else if (d.y() > d.z()) {
      return Y_AXIS;
    } else {
      return Z_AXIS;
    }
  }

  /**
   * Calculates the intersection of a ray with the bbox. (If there is an
   * intersection, then there will be two -- the entrance and exit points.)
   * This is from Pharr and Humphreys.
   *
   * @param r            the ray to shoot through the box
   * @param t0_out [out] the parameter of the ray at the first intersection
   *                     point (the entrance point)
   * @param t1_out [out] the parameter of the ray at the second intersection
   *                     point (the exit point)
   * @returns            true if intersections were found, false if no hit
   */
  inline bool intersect(const Ray &r, float* t0_out, float* t1_out) const {
    float t0 = 0.0f;
    float t1 = std::numeric_limits<float>::max();
    for (int i = 0; i < 3; ++i) {
      // Update interval for `i`th bounding box slab.
      float invRayDir = 1.0f / r.direction[i];
      float tNear = (lower[i] - r.origin[i]) * invRayDir;
      float tFar = (upper[i] - r.origin[i]) * invRayDir;
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

  friend std::ostream& operator<<(std::ostream& os, const BBox& b) {
    os << boost::format("<lower: %1%, upper: %2%>") % b.lower % b.upper;
    return os;
  }
};

/**
 * A bounding sphere.
 */
struct BSphere {
  Vec origin;
  float radius;

  BSphere(const Vec& o, float r = 0.0f) : origin(o), radius(r) {}

  BSphere(const BBox& b) {
    origin = (b.lower + b.upper) * 0.5f;
    radius = fabsf((b.upper - origin).norm());
  }

  inline bool contains(const Vec& v) const {
    return (v - origin).squaredNorm() <= (radius * radius);
  }
};

/**
 * Contains the information for a ray-object intersection.
 */
struct Intersection {
  Vec position; /**< The point of the intersection in 3D space. */
  Vec normal; /**< The normal of the surface at the intersection. */
  float distance; /**< The distance from the ray origin to the intersection. */

  /**
   * Constructs an intersection with no information.
   */
  Intersection()
    : position(0, 0, 0), normal(0, 0, 0),
      distance(std::numeric_limits<float>::max()) {}

  /**
   * Constructs an intersection with the given position, normal, and distance.   *
   * @param p the point of the intersection in 3D space
   * @param n the normal of the surface at the intersection
   * @param d the distance from the ray origin to the intersection
   */
  Intersection(const Vec& p, const Vec& n, float d)
    : position(p), normal(n), distance(d) {}

};
