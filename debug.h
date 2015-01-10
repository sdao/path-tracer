#pragma once
#include <cmath>
#include <cassert>
#include <iostream>
#include "math.h"

namespace debug {

  inline bool isNaN(const Vec& v) {
    return isnan(v.x()) || isnan(v.y()) || isnan(v.z());
  }

  inline void printNestedException(const std::exception& e, int level =  0)
  {
    if (level == 0) {
      std::cerr << "error: " << e.what() << "\n";
    } else {
      std::cerr << "       " << e.what() << "\n";
    }

    try {
      std::rethrow_if_nested(e);
    } catch(const std::exception& e) {
      printNestedException(e, level + 1);
    } catch(...) {}
  }

  template<typename T>
  inline T shouldNotReach(T x) {
    assert(false && "not reachable in normal circumstances");
    return x;
  }

}
