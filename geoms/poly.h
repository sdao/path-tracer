#pragma once
#include "../geom.h"
#include "../mem.h"

namespace geoms {
  
  class poly : public geom {
    struct point {
      vec position;
      vec normal;
      vec tangent;
      vec binormal;
      
      void computeTangents() {
        math::coordSystem(normal, &tangent, &binormal);
      }
    };
    
    inline point get(mem::id i) const {
      return (*pointLookup)[i];
    }
  
  public:
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
    
    static bool readPolyModel(
      material* m,
      std::string name,
      std::vector<geoms::poly::point>& pointLookup,
      std::vector<geoms::poly>& polys,
      vec offset
    );
  };
  
}
