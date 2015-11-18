#include "image.h"
#include <exception>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#define TINYEXR_IMPLEMENTATION
#include <tinyexr.h>

using boost::format;

Image::Image(long ww, long hh, long spp, float fw)
  : currentIteration(boost::extents[hh][ww][spp]),
    rawData(boost::extents[hh][ww]),
    channelR(hh * ww), channelG(hh * ww), channelB(hh * ww),
    w(ww), h(hh), samplesPerPixel(spp), filterWidth(fw)
{
  // Clear the data array.
  for (long y = 0; y < h; ++y) {
    for (long x = 0; x < w; ++x) {
      rawData[y][x] = Vec4(0, 0, 0, 0);
    }
  }
}

void Image::setSample(
  long x,
  long y,
  float ptX,
  float ptY,
  long idx,
  const Vec& color
) {
  Sample& s = currentIteration[y][x][idx];
  s.position = Vec2(ptX, ptY);
  s.color = color;
}

void Image::commitSamples() {
  for (const auto& row : currentIteration) {
    for (const auto& col : row) {
      for (const Sample& s : col) {
        float posX = s.position.x();
        float posY = s.position.y();

        long minX = math::clampAny(long(ceilf(posX - filterWidth)), 0l, w - 1);
        long maxX = math::clampAny(long(floorf(posX + filterWidth)), 0l, w - 1);
        long minY = math::clampAny(long(ceilf(posY - filterWidth)), 0l, h - 1);
        long maxY = math::clampAny(long(floorf(posY + filterWidth)), 0l, h - 1);

        for (long yy = minY; yy <= maxY; ++yy) {
          for (long xx = minX; xx <= maxX; ++xx) {
            Vec4& px = rawData[yy][xx];

            float weight = math::mitchellFilter(
              posX - float(xx),
              posY - float(yy),
              filterWidth
            );

            px[0] += s.color[0] * weight;
            px[1] += s.color[1] * weight;
            px[2] += s.color[2] * weight;
            px[3] += weight;
          }
        }
      }
    }
  }
}

void Image::writeToEXR(std::string fileName) {
  for (long y = 0; y != h; ++y) {
    for (long x = 0; x != w; ++x) {
      Vec4& px = rawData[y][x];

      long index = y * w + x;
      channelR[index] = px.x() / px.w();
      channelG[index] = px.y() / px.w();
      channelB[index] = px.z() / px.w();
    }
  }

  EXRImage image;
  InitEXRImage(&image);
  image.num_channels = 3;

  // Must be BGR(A) order, since most EXR viewers expect this channel order.
  const char* channel_names[] = { "B", "G", "R" };

  float* image_ptr[3] = {
    channelB.data(), // B
    channelG.data(), // G
    channelR.data()  // R
  };

  image.channel_names = channel_names;
  image.images = (unsigned char**)image_ptr;
  image.width = w;
  image.height = h;
  image.compression = TINYEXR_COMPRESSIONTYPE_NONE;

  image.pixel_types = new int[sizeof(int) * image.num_channels];
  image.requested_pixel_types = new int[sizeof(int) * image.num_channels];
  for (int i = 0; i < image.num_channels; i++) {
    image.pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT; // pixel type of input image
    image.requested_pixel_types[i] = TINYEXR_PIXELTYPE_HALF; // pixel type of output image to be stored in .EXR
  }

  const char* err;
  int ret = SaveMultiChannelEXRToFile(&image, fileName.c_str(), &err);

  delete[] image.pixel_types;
  delete[] image.requested_pixel_types;

  if (ret != 0) {
    throw std::runtime_error(
      str(format("Cannot read string property '%1%'") % std::string(err))
    );
  }
}
