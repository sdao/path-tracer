#include "embree.h"

#ifdef __SSE3__
#include <xmmintrin.h>
#include <pmmintrin.h>
#endif

#include "debug.h"
#include "geom.h"

bool Embree::embreeInited = false;
RTCDevice Embree::device = nullptr;

Embree::Embree(const std::vector<const Geom*>& o)
  : embreeObjStorage(o.size()), embreeObjLookup() {
  assert(embreeInited);
  scene = rtcDeviceNewScene(device, RTC_SCENE_STATIC, RTC_INTERSECT1);

  size_t i = 0;
  for (const Geom* g : o) {
    // Composite objects might become one object in Embree.
    EmbreeObj& eo = embreeObjStorage[i];
    g->makeEmbreeObject(scene, eo);
    embreeObjLookup[eo.geomId] = &eo;

    i++;
  }

  rtcCommit(scene);
}

void Embree::init() {
#ifdef __SSE3__
  _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
  _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
#endif

  device = rtcNewDevice(nullptr);
  embreeInited = true;
}

void Embree::exit() {
  rtcDeleteDevice(device);
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
  ray.mask = int(0xFFFFFFFF);
  ray.time = 0.0f;
  rtcIntersect(scene, ray);

  if (ray.geomID != int(RTC_INVALID_GEOMETRY_ID)) {
    const EmbreeObj* eo = embreeObjLookup.at(unsigned(ray.geomID));
    eo->isectCallback(eo, ray, isectOut);
    return eo->geom;
  }

  return nullptr;
}

bool Embree::intersectShadow(const Ray& r, float maxDist) const {
  RTCRay ray;
  ray.org[0] = r.origin.x();
  ray.org[1] = r.origin.y();
  ray.org[2] = r.origin.z();
  ray.dir[0] = r.direction.x();
  ray.dir[1] = r.direction.y();
  ray.dir[2] = r.direction.z();
  ray.tnear = 0.0f;
  ray.tfar = maxDist;
  ray.geomID = int(RTC_INVALID_GEOMETRY_ID);
  ray.primID = int(RTC_INVALID_GEOMETRY_ID);
  ray.instID = int(RTC_INVALID_GEOMETRY_ID);
  ray.mask = int(0xFFFFFFFF);
  ray.time = 0.0f;
  rtcOccluded(scene, ray);

  return ray.geomID == 0;
}
