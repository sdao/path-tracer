#include "poly.h"
#include <iostream>

geoms::poly::poly(
  material* m,
  mem::id a,
  mem::id b,
  mem::id c,
  std::vector<geoms::poly::point>* lookup
) : geom(m), pt0(a), pt1(b), pt2(c), pointLookup(lookup) {}

geoms::poly::poly(const geoms::poly& other)
  : geom(other.mat), pt0(other.pt0), pt1(other.pt1), pt2(other.pt2),
    pointLookup(other.pointLookup) {}

bool geoms::poly::intersect(const ray& r, intersection* isectOut) const {
  // Uses the Moller-Trumbore intersection algorithm.
  // See <http://en.wikipedia.org/wiki/
  // M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm> for more info.
  
  const point& pt0_data = get(pt0);
  const point& pt1_data = get(pt1);
  const point& pt2_data = get(pt2);
  
  const vec edge1 = pt1_data.position - pt0_data.position;
  const vec edge2 = pt2_data.position - pt0_data.position;
  
  const vec p = r.direction.cross(edge2);
  const float det = edge1.dot(p);
  
  if (math::isNearlyZero(det)) {
    return false; // No hit on plane.
  }
  
  const float invDet = 1.0f / det;
  const vec t = r.origin - pt0_data.position;
  
  const float u = t.dot(p) * invDet;
  if (u < 0.0f || u > 1.0f) {
    return false; // In plane but not triangle.
  }
  
  const vec q = t.cross(edge1);
  const float v = r.direction.dot(q) * invDet;
  if (v < 0.0f || (u + v) > 1.0f) {
    return false; // In plane but not triangle.
  }
  
  const float dist = edge2.dot(q) * invDet;
  if (dist < 0.0f) {
    return false; // In triangle but behind us.
  }
  
  const float w = 1.0f - u - v;
  
  if (isectOut) {
    intersection isect;
    isect.position = r.at(dist);
    isect.distance = dist;
    
    // See Pharr & Humphreys pp. 147-148.
    // The gist is that interpolation w/ barycentric coordinates will not
    // preserve the orthonormality of the basis vectors.
    isect.normal =
      w * pt0_data.normal + u * pt1_data.normal + v * pt2_data.normal;
    isect.tangent =
      w * pt0_data.tangent + u * pt1_data.tangent + v * pt2_data.tangent;
    isect.binormal = isect.tangent.cross(isect.normal);
    
    if (isect.binormal.squaredNorm() > 0.0f) {
      isect.binormal = isect.binormal.normalized();
      isect.tangent = isect.binormal.cross(isect.normal);
    } else {
      // Force recompute tangent and binormal.
      math::coordSystem(isect.normal, &isect.tangent, &isect.binormal);
    }
    
    *isectOut = isect;
  }
  
  return true;
}

bbox geoms::poly::bounds() const {
  bbox b(get(pt0).position, get(pt1).position);
  b.expand(get(pt2).position);
  
  return b;
}
