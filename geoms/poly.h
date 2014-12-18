#pragma once
#include "../geom.h"
#include "../mem.h"

namespace geoms {
  
  class poly : public geom {
  public:
    struct point {
      vec position;
      vec normal;
      vec tangent;
      vec binormal;
      
      inline void computeTangents() {
        math::coordSystem(normal, &tangent, &binormal);
      }
    };
    
    const mem::id pt0;
    const mem::id pt1;
    const mem::id pt2;
    std::vector<geoms::poly::point>* pointLookup; // Not owned by us.
    
    poly(
      material* m,
      mem::id a,
      mem::id b,
      mem::id c,
      std::vector<geoms::poly::point>* lookup
    );
    poly(const geoms::poly& other);
    
    virtual intersection intersect(const ray& r) const;
    virtual bbox bounds() const;
    
  private:
    inline const point get(mem::id i) const {
      return mem::ref(*pointLookup, i);
    }
  };
  
}
