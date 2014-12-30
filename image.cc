#include "image.h"

Image::Image(long ww, long hh, long spp, float fw)
  : currentIteration(boost::extents[hh][ww][spp]),
    rawData(boost::extents[hh][ww]),
    exrData(long(hh), long(ww)),
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
      Imf::Rgba& rgba = exrData[y][x];
      Vec4& px = rawData[y][x];

      rgba.r = px.x() / px.w();
      rgba.g = px.y() / px.w();
      rgba.b = px.z() / px.w();
      rgba.a = 1.0f;
    }
  }

  Imf::RgbaOutputFile file(fileName.c_str(), int(w), int(h), Imf::WRITE_RGBA);
  file.setFrameBuffer(&exrData[0][0], 1, size_t(w));
  file.writePixels(int(h));
}
