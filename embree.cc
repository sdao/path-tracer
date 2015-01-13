#include "embree.h"
#include "debug.h"
#include "geom.h"

bool Embree::embreeInited = false;

Embree::Embree(const std::vector<const Geom*>& o) : embreeObjs(), lights() {
  assert(embreeInited);
  scene = rtcNewScene(RTC_SCENE_STATIC, RTC_INTERSECT1);

  std::vector<const Geom*> refinedObjs;
  for (const Geom* g : o) {
    // Online refine lights.
    if (g->light) {
      g->refine(lights);
    }

    // Composite objects might become one object in Embree.
    EmbreeObj* eo = g->makeEmbreeObject(*this);
    embreeObjs[eo->geomId] = eo;
  }

  rtcCommit(scene);
}

void Embree::init() {
  rtcInit();
  embreeInited = true;
}

void Embree::exit() {
  rtcExit();
  embreeInited = false;
}

const Geom* Embree::intersect(const Ray& r, Intersection* isectOut) const {
  RTCRay ray;
  ray.org[0] = r.origin.x();
  ray.org[1] = r.origin.y();
  ray.org[2] = r.origin.z();
  ray.dir[0] = r.direction.x();
  ray.dir[1] = r.direction.y();
  ray.dir[2] = r.direction.z();
  ray.tnear = 0.0f;
  ray.tfar = math::VERY_BIG;
  ray.geomID = int(RTC_INVALID_GEOMETRY_ID);
  ray.primID = int(RTC_INVALID_GEOMETRY_ID);
  ray.instID = int(RTC_INVALID_GEOMETRY_ID);
  ray.mask = 0xFFFFFFFF;
  ray.time = 0.0f;
  rtcIntersect(scene, ray);

  if (ray.geomID != RTC_INVALID_GEOMETRY_ID) {
    const EmbreeObj* eo = embreeObjs.at(unsigned(ray.geomID));
    eo->isectCallback(eo, ray, isectOut);
    return eo->geom;
  }

  return nullptr;
}
