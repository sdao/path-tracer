#include "camera.h"
#include <tbb/tbb.h>
#include <limits>
#include <iostream>

#define MAX_DEPTH 10
#define RUSSIAN_ROULETTE_DEPTH 5

camera::camera(ray e, int ww, int hh, float ff = M_PI / 4.0f)
  : eye(e.unit()), w(ww), h(hh), fovx2(0.5f * ff),
    iters(0), data(h), exrData(hh, ww), rng(time(0)) {
  // Size of the image plane projected into world space
  // using the given fovx and cam focal length.
  float scale_right = 2.0f * glm::length(eye.direction) * tanf(fovx2);
  float scale_up = scale_right * ((float)h / float(w));

  // Corresponding vectors.
  up = -vec(0, 1, 0) * scale_up; // Flip the y-axis for image output!
  right = -glm::normalize(glm::cross(eye.direction, up)) * scale_right;

  // Image corner ray in world space.
  corner_ray = eye.direction - (0.5f * up) - (0.5f * right);

  // Prepare raw output data array.
  for (int y = 0; y < h; ++y) {
    data[y] = std::vector<dvec>(w);
    for (int x = 0; x < w; ++x) {
      data[y][x] = dvec(0);
    }
  }
}

geomptr camera::intersect(const ray& r,
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

void camera::renderOnce(std::vector<geomptr>& objs, std::string name) {
  iters++;
  std::cout << "Iteration " << iters << "\n";

  double newFrac = 1.0 / double(iters);
  double oldFrac = (double)(iters - 1.0) * newFrac;

  for (int y = 0; y < h; ++y) {
    std::mt19937 rowRng(intDist(rng));

    //tbb::parallel_for(size_t(0), size_t(w), [&](size_t x) {
    for (int x = 0; x < w; ++x) {
      dvec pxColor;
      for (int sy = -1; sy < 1; ++sy) {
        for (int sx = -1; sx < 1; ++sx) {
          float frac_y = (y + 0.5f * sy) / (h - 1.0f);
          float frac_x = (x + 0.5f * sx) / (w - 1.0f);

          lightray r(eye.origin, corner_ray + (up * frac_y) + (right * frac_x));
          vec sampColor(0);

          int depth = 0;
          bool died = false;
          while (!r.isZero() && !died) {
            depth++;
            died = false;

            // Do Russian Roulette if this ray is "old".
            if (depth > MAX_DEPTH) {
              died = true;
            } else if (depth > RUSSIAN_ROULETTE_DEPTH || r.isBlack()) {
              float rv = realDist(rowRng);
              float energy = r.energy();
              if (rv < energy) {
                // The ray lives.
                // Increase its energy to balance out probabilities.
                r.color = r.color * (1.0f / energy);
              } else {
                // The ray dies.
                died = true;
              }
            }

            // Bounce ray and kill if nothing hit.
            intersection isect;
            geomptr g = intersect(r, objs, &isect);

            if (g) {
              r = g->mat->propagate(r, isect, rowRng);
              sampColor = r.color;
            } else {
              died = true;
            }
          }

          // Black if ray died; fill in color otherwise.
          if (!died) {
            pxColor += dvec(sampColor.x, sampColor.y, sampColor.z);
          }
        }
      }

      pxColor *= 0.25;
      dvec &p = data[y][x];
      if (iters == 1) {
        p = pxColor;
      } else {
        p = p * oldFrac + pxColor * newFrac;
      }
    }
    //});
  }

  math::copyData(w, h, data, exrData);
  Imf::RgbaOutputFile file(name.c_str(), w, h, Imf::WRITE_RGBA);
  file.setFrameBuffer(&exrData[0][0], 1, w);
  file.writePixels(h);
}

void camera::renderInfinite(std::vector<geomptr>& objs, std::string name) {
  std:: cout << "Press Ctrl-c to quit\n";
  while (true) {
    renderOnce(objs, name);
  }
}
