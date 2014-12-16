#pragma once
#include <random>

class randomness {
  std::uniform_real_distribution<float> unitDist;
  std::uniform_int_distribution<int> intDist;
  std::uniform_int_distribution<unsigned> unsignedDist;

public:
  std::mt19937 rng;
  randomness(unsigned seed) : unitDist(), intDist(), rng(seed) {}

  inline int nextInt() { return intDist(rng); }
  inline unsigned nextUnsigned() { return unsignedDist(rng); }
  inline float nextUnitFloat() { return unitDist(rng); }
  inline float nextFloat(float max) {
    return max * unitDist(rng);
  }
  inline float nextFloat(float min, float max) {
    return min + (max - min) * unitDist(rng);
  }
};
