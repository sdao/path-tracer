#pragma once
#include <vector>
#include <map>
#include <functional>
#include "core.h"

class Geom;

extern "C" {
  #include <embree2/rtcore.h>
  #include <embree2/rtcore_ray.h>
}

class Embree {
  static bool embreeInited;

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
  };

  RTCScene scene;

  Embree(const std::vector<const Geom*>& o);

  static void init();
  static void exit();

  const Geom* intersect(const Ray& r, Intersection* isectOut) const;

private:
  std::map<unsigned, const EmbreeObj*> embreeObjs;
  std::vector<const Geom*> lights;
};
