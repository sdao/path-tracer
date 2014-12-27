#pragma once
#include <random>

/**
 * A unified RNG capable of generating random floating-point and integer
 * values. Not thread-safe; use a separate randomness object for each thread.
 */
class Randomness {
  /**
   * Uniform [0, 1) floating-point distribution.
   */
  std::uniform_real_distribution<float> unitDist;
  /**
   * Uniform distribution over all possible int's.
   */
  std::uniform_int_distribution<int> intDist;
  /**
   * Uniform distribution over all possible unsigned's.
   */
  std::uniform_int_distribution<unsigned> unsignedDist;
  /**
   * Standard Gaussian (normal) distribution.
   */
  std::normal_distribution<float> normalDist;
  /**
   * The engine used internally for the RNG.
   */
  std::mt19937 rng;

  /**
   * Returns a seed based on true device randomness.
   */
  static inline unsigned createSeed() {
    std::uniform_int_distribution<unsigned> seedDist;
    std::random_device randDevice;

    return seedDist(randDevice);
  }

public:
  /**
   * Constructs a randomness object from a truly random seed.
   */
  Randomness()
    : unitDist(), intDist(), unsignedDist(), normalDist(), rng(createSeed()) {}

  /**
   * Constructs a randomness object from the given seed.
   */
  Randomness(unsigned seed)
    : unitDist(), intDist(), unsignedDist(), rng(seed) {}

  /**
   * Samples a random int.
   */
  inline int nextInt() { return intDist(rng); }

  /**
   * Samples a random unsigned.
   */
  inline unsigned nextUnsigned() { return unsignedDist(rng); }

  /**
   * Samples a random float between 0 (inclusive) and 1 (exclusive).
   */
  inline float nextUnitFloat() { return unitDist(rng); }

  /**
   * Samples a random float between 0 (inclusive) and max (exclusive).
   */
  inline float nextFloat(float max) {
    return max * unitDist(rng);
  }

  /**
   * Samples a random float between min (inclusive) and max (exclusive).
   */
  inline float nextFloat(float min, float max) {
    return min + (max - min) * unitDist(rng);
  }

  /**
   * Samples a normally-distributed float with mean 0 and standard deviation 1.
   */
  inline float nextNormalFloat() { return normalDist(rng); }
};
