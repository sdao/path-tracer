#pragma once
#include <vector>

class Geom;
struct Ray;
struct Intersection;

class Accelerator {
public:
   virtual ~Accelerator();

  /**
   * Determines what object (if any) a given ray intersects.
   *
   * @param r              the ray to send through the k-d tree
   * @param isectOut [out] the intersection information if the ray hit some
   *                       geometry, otherwise unmodified; the pointer must not
   *                       be null
   * @returns              the geom that was hit, or nullptr if none was hit
   */
  virtual const Geom* intersect(const Ray& r, Intersection* isectOut) const = 0;

  /**
   * Determines if any object intersects the given shadow ray within a maximum
   * distance.
   *
   * @param r       the shadow ray to send through the k-d tree
   * @param maxDist the maximum distance to check for intersections
   * @returns       true if any geom hit within maxDist, otherwise false
   */
  virtual bool intersectShadow(const Ray& r, float maxDist) const = 0;
};
