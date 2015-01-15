#pragma once
#include <vector>
#include <map>
#include <functional>
#include "core.h"
#include "accelerator.h"

class Geom;

#include <embree2/rtcore.h>
#include <embree2/rtcore_ray.h>

class Embree : public Accelerator {
  static bool embreeInited;
  RTCScene scene;

public:
  struct EmbreeVert { float x, y, z, a; };
  struct EmbreeTri { int v0, v1, v2; };
  struct EmbreeObj {
    using IntersectionCallback =
      std::function<void(const EmbreeObj*, const RTCRay&, Intersection*)>;

    const Geom* geom;
    unsigned geomId;
    IntersectionCallback isectCallback;
    
    EmbreeObj(const Geom* g, unsigned i, IntersectionCallback c)
      : geom(g), geomId(i), isectCallback(c) {}
    EmbreeObj()
      : geom(nullptr), geomId(RTC_INVALID_GEOMETRY_ID), isectCallback() {}
  };

  Embree(const std::vector<const Geom*>& o);

  static void init();
  static void exit();

  virtual const Geom* intersect(
    const Ray& r,
    Intersection* isectOut
  ) const override;
  virtual bool intersectShadow(const Ray& r, float maxDist) const override;

private:
  std::vector<EmbreeObj> embreeObjStorage;
  std::map<unsigned, const EmbreeObj*> embreeObjLookup;
};
