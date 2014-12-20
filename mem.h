#pragma once

namespace mem {

  /**
   * Represents an invalid array/vector index.
   * All valid indices must be lower than this index.
   */
  static constexpr size_t ID_INVALID = std::numeric_limits<size_t>::max();
  
  /**
   * Represents a typesafe array/vector index.
   */
  union id {
    size_t val; /**< The array/vector index stored in this id. */
    
    /**
     * Constructs an id from the specified array/vector index.
     */
    id(size_t i = ID_INVALID) : val(i) {}
    
    inline id& operator=(size_t other) {
      val = other;
      return *this;
    }
    
    /**
     * Creates another id from the current id by adding an unsigned offset to
     * its array/vector index.
     */
    inline id offset(size_t ofs) const {
      return id(val + ofs);
    }
    
    /**
     * Whether the current id represents a valid array/vector index.
     */
    inline bool isValid() const {
      return val < ID_INVALID;
    }
  };
  
  /**
   * Returns a reference to the item in the vector corresponding to the given
   * id.
   */
  template<typename T>
  inline T& ref(std::vector<T>& vector, id item) {
    return vector[item.val];
  }
  
  /**
   * Returns a constant reference to the item in the vector corresponding to the
   * given id.
   */
  template<typename T>
  inline const T& refConst(const std::vector<T>& vector, id item) {
    return vector[item.val];
  }
  
}
