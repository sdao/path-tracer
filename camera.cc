#include "camera.h"
#include <limits>
#include <iostream>

#define MAX_DEPTH 20 // Slightly biased, but should be high enough.
#define RUSSIAN_ROULETTE_DEPTH 5
#define SAMPLES_PER_PIXEL 4
#define PIXELS_PER_SAMPLE 0.25f

camera::camera(ray e, size_t ww, size_t hh, float ff)
  : eye(e), w(ww), h(hh), fovx2(0.5f * ff),
    masterRng(unsigned(time(0))), rowSeeds(hh),
    data(hh), exrData(long(hh), long(ww)), iters(0)
{
  // Size of the image plane projected into world space
  // using the given fovx and cam focal length.
  float scale_right = 2.0f * glm::length(eye.direction) * tanf(fovx2);
  float scale_up = scale_right * (float(h) / float(w));

  // Corresponding vectors.
  up = -vec(0, 1, 0) * scale_up; // Flip the y-axis for image output!
  right = -glm::normalize(glm::cross(eye.direction, up)) * scale_right;

  // Image corner ray in world space.
  corner_ray = eye.direction - (0.5f * up) - (0.5f * right);

  // Prepare raw output data array.
  for (size_t y = 0; y < h; ++y) {
    data[y] = std::vector<dvec>(w);
    for (size_t x = 0; x < w; ++x) {
      data[y][x] = dvec(0);
    }
  }
}

geomptr camera::intersect(
  const ray& r,
  const std::vector<geomptr>& objs,
  intersection* isect_out
) const {
  intersection winner_isect;
  geomptr winner;

  for (auto& obj : objs) {
    intersection isect = obj->intersect(r);
    if (isect.hit()
      && (!winner_isect.hit() || isect.distance < winner_isect.distance)) {
      winner_isect = isect;
      winner = obj;
    }
  }

  if (isect_out) {
    *isect_out = winner_isect;
  }

  return winner;
}

void camera::renderOnce(const std::vector<geomptr>& objs, std::string name) {
  iters++;
  std::cout << "Iteration " << iters << "\n";

  double newFrac = 1.0 / double(iters);
  double oldFrac = double(iters - 1.0) * newFrac;

  for (size_t y = 0; y < h; ++y) {
    rowSeeds[y] = masterRng.nextUnsigned();
  }

  //tbb::parallel_for(size_t(0), size_t(h), [&](size_t y) {
  for (size_t y = 0; y < h; ++y) {
    randomness rng(rowSeeds[y]);

    for (size_t x = 0; x < w; ++x) {
      dvec pxColor;
      for (unsigned samps = 0; samps < SAMPLES_PER_PIXEL; ++samps) {
        float frac_y =
          (float(y) - 0.5f + rng.nextUnitFloat()) / (float(h) - 1.0f);
        float frac_x =
          (float(x) - 0.5f + rng.nextUnitFloat()) / (float(w) - 1.0f);

        lightray r(
          eye.origin,
          glm::normalize(corner_ray + (up * frac_y) + (right * frac_x))
        );

        unsigned depth = 0;
        while (!r.isZeroLength()) {
          depth++;

          // Do Russian Roulette if this ray is "old".
          if (depth > MAX_DEPTH) {
            r.kill();
            break;
          } else if (depth > RUSSIAN_ROULETTE_DEPTH || r.isBlack()) {
            float rv = rng.nextUnitFloat();
            float probLive = math::clamp(r.energy());
            if (rv < probLive) {
              // The ray lives (more energy = more likely to live).
              // Increase its energy to balance out probabilities.
              r.color = r.color / probLive;
            } else {
              // The ray dies.
              r.kill();
              break;
            }
          }

          // Bounce ray and kill if nothing hit.
          intersection isect;
          geomptr g = intersect(r, objs, &isect);

          if (g) {
            r = g->mat->propagate(r, isect, rng);
          } else {
            r.kill();
            break;
          }
        }

        // Black if ray died; fill in color otherwise.
        pxColor += dvec(r.color.x, r.color.y, r.color.z);
      }

      pxColor *= PIXELS_PER_SAMPLE;

      dvec &p = data[y][x];
      if (iters == 1) {
        p = pxColor;
      } else {
        p = p * oldFrac + pxColor * newFrac;
      }
    } // end of x-for loop
  } // end of y-for loop
  //});

  math::copyData(w, h, data, exrData);
  Imf::RgbaOutputFile file(name.c_str(), int(w), int(h), Imf::WRITE_RGBA);
  file.setFrameBuffer(&exrData[0][0], 1, w);
  file.writePixels(int(h));
}

void camera::renderInfinite(const std::vector<geomptr>& objs,
    std::string name) {
  std:: cout << "Press Ctrl-c to quit\n";
  while (true) {
    renderOnce(objs, name);
  }

  /* Will not return. */
}
