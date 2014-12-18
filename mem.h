#pragma once

namespace mem {

  static constexpr size_t ID_INVALID = std::numeric_limits<size_t>::max();
  
  union id {
    size_t val;
    
    id(size_t i = ID_INVALID) : val(i) {}
    
    inline id& operator=(size_t other) {
      val = other;
      return *this;
    }
    
    inline id offset(size_t ofs) const {
      return id(val + ofs);
    }
    
    inline bool isValid() const {
      return val < ID_INVALID;
    }
  };
  
  template<typename T>
  inline T& ref(std::vector<T>& vector, id item) {
    return vector[item.val];
  }
  
  template<typename T>
  inline const T& refConst(const std::vector<T>& vector, id item) {
    return vector[item.val];
  }
  
}
