#pragma once

/**
 * Represents a typesafe array/vector index.
 */
struct ID {
  /**
   * An invalid array/vector index.
   * All valid indices must be lower than this index.
   */
  static constexpr size_t ID_INVALID = std::numeric_limits<size_t>::max();

  /**
   * The array/vector index stored in this id.
   */
  size_t val;

  /**
   * Constructs an id from the specified array/vector index.
   */
  ID(size_t i = ID_INVALID) : val(i) {}

  inline ID& operator=(size_t other) {
    val = other;
    return *this;
  }

  /**
   * Creates another id from the current id by adding an unsigned offset to
   * its array/vector index.
   */
  inline ID offset(size_t ofs) const {
    return ID(val + ofs);
  }

  /**
   * Whether the current id represents a valid array/vector index.
   */
  inline bool isValid() const {
    return val < ID_INVALID;
  }

  /**
   * Returns a reference to the item in the vector corresponding to this ID.
   */
  template<typename T>
  inline T& ref(std::vector<T>& vector) const {
    return vector[val];
  }

  /**
   * Returns a constant reference to the item in the vector corresponding to
   * this ID.
   */
  template<typename T>
  inline const T& refConst(const std::vector<T>& vector) const {
    return vector[val];
  }
};
