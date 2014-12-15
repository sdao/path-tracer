#pragma once
#include <random>

class randomness {
  std::normal_distribution<float> normalDist;
  std::uniform_real_distribution<float> unitDist;
  std::uniform_int_distribution<int> intDist;

public:
  std::mt19937 rng;
  randomness(int seed) : normalDist(), unitDist(), intDist(), rng(seed) {}

  inline int nextInt() { return intDist(rng); }
  inline float nextNormalFloat() { return normalDist(rng); }
  inline float nextUnitFloat() { return unitDist(rng); }
};
