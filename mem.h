#pragma once

namespace mem {
  typedef size_t id;
  static constexpr id ID_INVALID = std::numeric_limits<id>::max();
  
  inline bool isValidId(id i) {
    return i < ID_INVALID;
  }
}
