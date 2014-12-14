#include <OpenEXR/ImfRgbaFile.h>
#include <OpenEXR/ImfArray.h>
#include <tbb/tbb.h>
#include <limits>
#include <vector>
#include <memory>
#include <random>
#include <iostream>
#include "math.h"
#include "material.h"
#include "sphere.h"
#include "plane.h"

geomptr intersect(const ray& r,
                  std::vector<geomptr> objs,
                  intersection* isect_out = nullptr) {
  intersection winner_isect;
  geomptr winner;

  for (int i = 0; i < objs.size(); i++) {
    intersection isect = objs[i]->intersect(r);
    if (isect.hit()
      && (!winner_isect.hit() || isect.distance < winner_isect.distance)) {
      winner_isect = isect;
      winner = objs[i];
    }
  }

  if (isect_out) {
    *isect_out = winner_isect;
  }

  return winner;
}

void render(int w, int h, Imf::Array2D<Imf::Rgba>& pixels) {
  materialptr red = std::make_shared<idealdiffuse>(vec(1, 0.3, 0.3));
  materialptr white = std::make_shared<idealdiffuse>(vec(1, 1, 1));
  materialptr blue = std::make_shared<idealdiffuse>(vec(0.3, 0.3, 1));
  materialptr emit = std::make_shared<idealemitter>(vec(4, 4, 4));
  materialptr spec = std::make_shared<idealspecular>();

  std::vector<geomptr> objs;
  objs.push_back(std::make_shared<sphere>(vec(0, -8, 0), 4.0f, spec));
  objs.push_back(std::make_shared<plane>(vec(0, -20, 0), vec(0, 1, 0), red)); // bottom
  objs.push_back(std::make_shared<plane>(vec(0, 20, 0), vec(0, -1, 0), red)); // top
  objs.push_back(std::make_shared<plane>(vec(0, 0, -50), vec(0, 0, 1), blue)); // back
  objs.push_back(std::make_shared<plane>(vec(-20, 0, 0), vec(1, 0, 0), white)); // left
  objs.push_back(std::make_shared<plane>(vec(20, 0, 0), vec(-1, 0, 0), white)); // right
  objs.push_back(std::make_shared<sphere>(vec(0, 48, -30), 30.0f, emit)); // light

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
    for (int x = 0; x < w; ++x) {
      Imf::Rgba &p = pixels[y][x];
      p.r = 0.0f;
      p.g = 0.0f;
      p.b = 0.0f;
      p.a = 1.0f;
    }
  }

  std::mt19937 rng(time(0));
  std::uniform_int_distribution<int> dist;
  int iters = 0;
  while (true) {
    iters++;
    std::cout << "Iteration " << iters << "\n";

    float newFrac = 1.0f / float(iters);
    float oldFrac = (float)(iters - 1.0f) * newFrac;

    for (int y = 0; y < h; ++y) {
      float frac_y = y / ((float)h - 1.0f);
      std::mt19937 rowRng(dist(rng));

      tbb::parallel_for(size_t(0), size_t(w), [&](size_t x) {
        float frac_x = x / ((float)w - 1.0f);

        lightray r(cam.origin, corner_dir + (up * frac_y) + (right * frac_x));
        vec color(0);

        int limit = 10;
        while (!r.isZero()) {
          limit--;

          intersection isect;
          geomptr g = intersect(r, objs, &isect);

          if (g && limit >= 0) {
            r = g->mat->propagate(r, isect, rowRng);
            color = r.color;
          } else {
            color = vec(0); // Hit empty space, which isn't a light source.
            break;
          }
        }

        Imf::Rgba &p = pixels[y][x];
        if (iters == 1) {
          p.r = color.x;
          p.g = color.y;
          p.b = color.z;
        } else {
          p.r = p.r * oldFrac + color.x * newFrac;
          p.g = p.g * oldFrac + color.y * newFrac;
          p.b = p.b * oldFrac + color.z * newFrac;
        }
      });
    }

    Imf::RgbaOutputFile file("/Users/Steve/Desktop/sample.exr", w, h,
    Imf::WRITE_RGBA);
    file.setFrameBuffer(&pixels[0][0], 1, w);
    file.writePixels(h);
  }
}

int main() {
  int w = 512;
  int h = 384;

  Imf::Array2D<Imf::Rgba> pixels(h, w);
  render(512, 384, pixels);
/*
  Imf::RgbaOutputFile file("/Users/Steve/Desktop/sample.exr", w, h,
    Imf::WRITE_RGBA);
  file.setFrameBuffer(&pixels[0][0], 1, w);
  file.writePixels(h);
*/
  return 0;
}
