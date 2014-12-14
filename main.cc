#include <OpenEXR/ImfRgbaFile.h>
#include <OpenEXR/ImfArray.h>
#include <limits>
#include <vector>
#include <memory>
#include "math.h"
#include "material.h"
#include "sphere.h"
#include "plane.h"

geomptr intersect(const ray& r,
               std::vector<geomptr> objs,
               float* dist_out = nullptr) {
  float dist = std::numeric_limits<float>::max();
  geomptr winner;

  for (int i = 0; i < objs.size(); i++) {
    intersection isect = objs[i]->intersect(r);
    if (isect.hit() && isect.distance < dist) {
      dist = isect.distance;
      winner = objs[i];
    }
  }

  if (dist_out) {
    *dist_out = dist;
  }

  return winner;
}

void render(int w, int h, Imf::Array2D<Imf::Rgba>& pixels) {
  materialptr red = std::make_shared<testmaterial>(vec(1, 0, 0));
  materialptr green = std::make_shared<testmaterial>(vec(0, 1, 0));
  materialptr blue = std::make_shared<testmaterial>(vec(0, 0, 1));
  materialptr white = std::make_shared<testmaterial>(vec(1, 1, 1));

  std::vector<geomptr> objs;
  objs.push_back(std::make_shared<sphere>(vec(0, 0, 0), 2.0f, green));
  objs.push_back(std::make_shared<sphere>(vec(0, -4, 0), 2.0f, red));
  objs.push_back(std::make_shared<plane>(vec(0, -20, 0), vec(0, 1, 0), red)); // bottom
  objs.push_back(std::make_shared<plane>(vec(0, 20, 0), vec(0, -1, 0), red)); // top
  objs.push_back(std::make_shared<plane>(vec(0, 0, -50), vec(0, 0, 1), blue)); // back
  objs.push_back(std::make_shared<plane>(vec(-20, 0, 0), vec(1, 0, 0), green)); // left
  objs.push_back(std::make_shared<plane>(vec(20, 0, 0), vec(-1, 0, 0), green)); // right
  objs.push_back(std::make_shared<sphere>(vec(0, 48, -30), 30.0f, white)); // light

  // origin = cam origin
  // direction = vector to focal point (unnormalized)
  ray cam(vec(0, 0, 50), glm::normalize(vec(0, 0, -100)));
  vec fwd = glm::normalize(cam.direction);

  float fovx = M_PI / 4.0f;
  float fovx2 = 0.5f * fovx;

  // Size of the image plane projected into world space
  // using the given fovx and cam focal length.
  float scale_right = 2.0f * glm::length(cam.direction) * tanf(fovx2);
  float scale_up = scale_right * ((float)h / float(w));

  // Corresponding vectors.
  vec up = -vec(0, 1, 0) * scale_up; // Flip the y-axis for image output!
  vec right = -glm::normalize(glm::cross(fwd, up)) * scale_right;

  // Image corner ray in world space.
  vec corner_dir = cam.direction - (0.5f * up) - (0.5f * right);

  for (int y = 0; y < h; ++y) {
    float frac_y = y / ((float)h - 1.0f);

    for (int x = 0; x < w; ++x) {
      float frac_x = x / ((float)w - 1.0f);

      ray r(cam.origin, corner_dir + (up * frac_y) + (right * frac_x));
      geomptr g = intersect(r, objs);
      vec color;

      if (g) {
        color = g->mat->debug_color;
      } else {
        color = vec(0);
      }

      Imf::Rgba &p = pixels[y][x];
      p.r = color.x;
      p.g = color.y;
      p.b = color.z;
      p.a = 1.0f;
    }
  }
}

int main() {
  int w = 512;
  int h = 384;

  Imf::Array2D<Imf::Rgba> pixels(h, w);
  render(512, 384, pixels);

  Imf::RgbaOutputFile file("/Users/Steve/Desktop/sample.exr", w, h,
    Imf::WRITE_RGBA);
  file.setFrameBuffer(&pixels[0][0], 1, w);
  file.writePixels(h);

  return 0;
}
