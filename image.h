#pragma once
#include <OpenEXR/ImfRgbaFile.h>
#include <OpenEXR/ImfArray.h>
#include <boost/multi_array.hpp>
#include "math.h"

class Image {

  struct Sample {
    Vec2 position;
    Vec color;

    Sample() : position(0, 0), color(0, 0, 0) {}
  };

  typedef boost::multi_array<Sample, 3> SampleArray;
  typedef boost::multi_array<Vec4, 2> PixelArray;

  /** The samples from the current iteration. */
  SampleArray currentIteration;

  /** The raw sampled colors and weights. */
  PixelArray rawData;

  /** The array used for writing to an OpenEXR file. */
  Imf::Array2D<Imf::Rgba> exrData;

public:
  /**
   * Default width (radius) of the filter kernel.
   */
  static constexpr float DEFAULT_FILTER_WIDTH = 2.0f;

  /**
   * The default number of samples to take per pixel per iteration.
   */
  static constexpr long DEFAULT_SAMPLES_PER_PIXEL = 4;

  const long w; /**< The width of the output image. */
  const long h; /**< The height of the output image. */
  const long samplesPerPixel; /**< Samples per pixel per iteration. */
  const float filterWidth; /**< The width (radius) of the filter kernel. */

  /**
   * Constructs a new image.
   *
   * @param ww  the width of the image
   * @param hh  the height of the image
   * @param spp the number of samples per pixel per iteration
   * @param fw  the width (radius) of the filter kernel
   */
  Image(
    long ww,
    long hh,
    long spp = DEFAULT_SAMPLES_PER_PIXEL,
    float fw = DEFAULT_FILTER_WIDTH
  );

  /**
   * Sets the specified sample for the current iteration. The sample will
   * not be applied to the image until Image::commitSamples is called.
   * This is thread-safe if no two threads call this function with the same
   * arguments (x, y, idx) at the same time. Otherwise, it is NOT thread-safe.
   *
   * @param x     the x-coordinate of the pixel for which the sample was taken
   * @param y     the y-coordinate of the pixel for which the sample was taken
   * @param ptX   the actual x-position of the sample, if jittered
   * @param ptY   the actual y-position of the sample, if jittered
   * @param idx   the index of the sample, 0 <= idx < samplesPerPixel
   * @param color the color of the sample
   */
  void setSample(
    long x,
    long y,
    float ptX,
    float ptY,
    long idx,
    const Vec& color
  );

  /**
   * Takes the currently-set samples, filters their values, and adds them to
   * the image. This is NOT thread-safe.
   */
  void commitSamples();

  /**
   * Writes the currently-committed image to an OpenEXR file on disk.
   */
  void writeToEXR(std::string fileName);
};
