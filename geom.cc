#include "geom.h"

Geom::Geom(const Material* m, const AreaLight* l) : mat(m), light(l) {}

Geom::Geom(const Parser& p) : Geom(p.getMaterial("mat"), p.getLight("light")) {}

Geom::~Geom() {}

void Geom::refine(std::vector<const Geom*>& refined) const {
  refined.push_back(this);
}

BSphere Geom::boundSphere() const {
  return BSphere(bounds());
}
