#pragma once
#include <cmath>
#include <cassert>
#include "math.h"

namespace debug {

  inline bool isNaN(const Vec& v) {
    return isnan(v.x()) || isnan(v.y()) || isnan(v.z());
  }

  template<typename T>
  inline T shouldNotReach(T x) {
    assert(false && "not reachable in normal circumstances");
    return x;
  }

}
