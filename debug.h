#pragma once
#include <cmath>
#include <cassert>
#include <iostream>
#include "math.h"

namespace debug {

  inline bool isNaN(const Vec& v) {
    return std::isnan(v.x()) || std::isnan(v.y()) || std::isnan(v.z());
  }

  inline void printNestedException(const std::exception& e)
  {
    std::cerr << e.what() << "\n";

    try {
      std::rethrow_if_nested(e);
    } catch(const std::exception& e) {
      printNestedException(e);
    } catch(...) {}
  }

  template<typename T>
  inline T shouldNotReach(T x) {
    assert(false && "not reachable in normal circumstances");
    return x;
  }

}
