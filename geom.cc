#include "geom.h"
#include "debug.h"

Geom::Geom(const Material* m, const AreaLight* l) : mat(m), light(l) {}

Geom::Geom(const Node& n) : Geom(n.getMaterial("mat"), n.getLight("light")) {}

Geom::~Geom() {}

void Geom::refine(std::vector<const Geom*>& refined) const {
  refined.push_back(this);
}

BSphere Geom::boundSphere() const {
  return BSphere(bounds());
}

void Geom::embreeBoundsFunc(void* user, size_t /* i */, RTCBounds& bounds) {
  const Embree::EmbreeObj* eo = reinterpret_cast<Embree::EmbreeObj*>(user);
  BBox b = eo->geom->bounds();
  bounds.lower_x = b.lower.x();
  bounds.lower_y = b.lower.y();
  bounds.lower_z = b.lower.z();
  bounds.upper_x = b.upper.x();
  bounds.upper_y = b.upper.y();
  bounds.upper_z = b.upper.z();
}

void Geom::embreeIntersectFunc(void* user, RTCRay& ray, size_t i) {
  const Embree::EmbreeObj* eo = reinterpret_cast<Embree::EmbreeObj*>(user);
  Ray r(
    Vec(ray.org[0], ray.org[1], ray.org[2]),
    Vec(ray.dir[0], ray.dir[1], ray.dir[2])
  );
  Intersection isect;
  if (eo->geom->intersect(r, &isect)) {
    ray.u = 0.0f;
    ray.v = 0.0f;
    ray.tfar = isect.distance;
    ray.geomID = int(eo->geomId);
    ray.primID = int(i);
    ray.Ng[0] = isect.normal.x();
    ray.Ng[1] = isect.normal.y();
    ray.Ng[2] = isect.normal.z();
  }
}

void Geom::embreeOccludedFunc(void* user, RTCRay& ray, size_t /* i */) {
  const Embree::EmbreeObj* eo = reinterpret_cast<Embree::EmbreeObj*>(user);
  Ray r(
    Vec(ray.org[0], ray.org[1], ray.org[2]),
    Vec(ray.dir[0], ray.dir[1], ray.dir[2])
  );
  Intersection isect;
  if (eo->geom->intersectShadow(r, ray.tfar)) {
    ray.geomID = 0;
  }
}

void Geom::embreeIntersectCallback(
  const Embree::EmbreeObj* eo,
  const RTCRay& ray,
  Intersection* isectOut
) {
  isectOut->position = Vec(
    ray.org[0] + ray.dir[0] * ray.tfar,
    ray.org[1] + ray.dir[1] * ray.tfar,
    ray.org[2] + ray.dir[2] * ray.tfar
  );
  isectOut->distance = ray.tfar;
  isectOut->normal = Vec(ray.Ng[0], ray.Ng[1], ray.Ng[2]).normalized();
}

Embree::EmbreeObj* Geom::makeEmbreeObject(Embree& embree) const {
  unsigned geomId = rtcNewUserGeometry(embree.scene, 1);
  Embree::EmbreeObj* eo = new Embree::EmbreeObj(
    this, geomId, &Geom::embreeIntersectCallback
  );

  rtcSetUserData(embree.scene, geomId, eo);
  rtcSetBoundsFunction(embree.scene, geomId, &Geom::embreeBoundsFunc);
  rtcSetIntersectFunction(embree.scene, geomId, &Geom::embreeIntersectFunc);
  rtcSetOccludedFunction(embree.scene, geomId, &Geom::embreeOccludedFunc);

  return eo;
}
