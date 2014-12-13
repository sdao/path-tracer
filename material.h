#pragma once
#include "math.h"

enum material_type {
  DIFFUSE = 0
};

struct material {
  material_type type;
  vec diffuse_color;
  vec emission;

  material() : type(DIFFUSE), diffuse_color(0.0f), emission(0.0f) {}
  material(material_type t, vec d, vec e = vec(0.0))
    : type(t), diffuse_color(d), emission(e) {}
};
