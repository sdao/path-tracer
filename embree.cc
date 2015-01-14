#include "embree.h"
#include "debug.h"
#include "geom.h"

bool Embree::embreeInited = false;

Embree::Embree(const std::vector<const Geom*>& o)
  : embreeObjStorage(o.size()), embreeObjLookup(), lights() {
  assert(embreeInited);
  scene = rtcNewScene(RTC_SCENE_STATIC, RTC_INTERSECT1);

  size_t i = 0;
  for (const Geom* g : o) {
    // Only refine lights; normal objects don't need refinement.
    if (g->light) {
      g->refine(lights);
    }

    // Composite objects might become one object in Embree.
    EmbreeObj& eo = embreeObjStorage[i];
    g->makeEmbreeObject(scene, eo);
    embreeObjLookup[eo.geomId] = &eo;

    i++;
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

const std::vector<const Geom*>& Embree::getLights() const {
  return lights;
}
