#include "geom.h"

Geom::Geom(const Material* m, const AreaLight* l) : mat(m), light(l) {}

Geom::~Geom() {}
