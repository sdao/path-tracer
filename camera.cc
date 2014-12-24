#include "camera.h"
#include <iostream>
#include <chrono>

namespace chrono = std::chrono;

camera::camera(ray e, size_t ww, size_t hh, float ff)
  : eye(ray(e.origin, e.direction.normalized())), fovx2(0.5f * ff), masterRng(),
    rowSeeds(hh), data(hh), exrData(long(hh), long(ww)), w(ww), h(hh), iters(0)
{
  // Size of the image plane projected into world space
  // using the given fovx and cam focal length.
  float scaleRight = 2.0f * eye.direction.norm() * tanf(fovx2);
  float scaleUp = scaleRight * (float(h) / float(w));

  // Corresponding vectors.
  up = -vec(0, 1, 0) * scaleUp; // Flip the y-axis for image output!
  right = -eye.direction.cross(up).normalized() * scaleRight;

  // Image corner ray in world space.
  cornerRay = eye.direction - (0.5f * up) - (0.5f * right);

  // Prepare raw output data array.
  for (size_t y = 0; y < h; ++y) {
    data[y] = std::vector<dvec>(w);
    for (size_t x = 0; x < w; ++x) {
      data[y][x] = dvec(0, 0, 0);
    }
  }
}

void camera::renderOnce(const kdtree& kdt, std::string name) {
  iters++;
  std::cout << "Iteration " << iters;
  chrono::steady_clock::time_point startTime = chrono::steady_clock::now();

  double newFrac = 1.0 / double(iters);
  double oldFrac = double(iters - 1.0) * newFrac;

  for (size_t y = 0; y < h; ++y) {
    rowSeeds[y] = masterRng.nextUnsigned();
  }

  tbb::parallel_for(size_t(0), size_t(h), [&](size_t y) {
  //for (size_t y = 0; y < h; ++y) {
    randomness rng(rowSeeds[y]);

    for (size_t x = 0; x < w; ++x) {
      dvec pxColor(0, 0, 0);
      for (int samps = 0; samps < SAMPLES_PER_PIXEL; ++samps) {
        float frac_y =
          (float(y) - 0.5f + rng.nextUnitFloat()) / (float(h) - 1.0f);
        float frac_x =
          (float(x) - 0.5f + rng.nextUnitFloat()) / (float(w) - 1.0f);

        lightray r(
          eye.origin,
          (cornerRay + (up * frac_y) + (right * frac_x)).normalized()
        );

        int depth = 0;
        while (!r.isZeroLength()) {
          depth++;

          // Do Russian Roulette if this ray is "old".
          if (depth > RUSSIAN_ROULETTE_DEPTH_2) {
            float rv = rng.nextUnitFloat();
            float probLive = 0.1f;
            if (rv < probLive) {
              // The ray lives (more energy = more likely to live).
              // Increase its energy to balance out probabilities.
              r.color = r.color / probLive;
            } else {
              // The ray dies.
              r.kill();
              break;
            }
          } else if (depth > RUSSIAN_ROULETTE_DEPTH_1 || r.isBlack()) {
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
          geom* g = kdt.intersect(r, &isect);

          if (g) {
            r = g->mat->propagate(r, isect, rng);
          } else {
            r.kill();
            break;
          }
        }

        // Black if ray died; fill in color otherwise.
        pxColor += dvec(r.color.x(), r.color.y(), r.color.z());
      }

      pxColor *= PIXELS_PER_SAMPLE;

      dvec &p = data[y][x];
      if (iters == 1) {
        p = pxColor;
      } else {
        p = p * oldFrac + pxColor * newFrac;
      }
    } // end of x-for loop
  //} // end of y-for loop
  });

  math::copyData(w, h, data, exrData);
  Imf::RgbaOutputFile file(name.c_str(), int(w), int(h), Imf::WRITE_RGBA);
  file.setFrameBuffer(&exrData[0][0], 1, w);
  file.writePixels(int(h));

  chrono::steady_clock::time_point endTime = chrono::steady_clock::now();
  chrono::duration<double> runTime =
    chrono::duration_cast<chrono::duration<double>>(endTime - startTime);
  std::cout << " [" << runTime.count() << " seconds]\n";
}

void camera::renderMultiple(
  const kdtree& kdt,
  std::string name,
  int iterations
) {
  if (iterations < 0) {
    // Run forever.
    std::cout << "Rendering infinitely, press Ctrl-c to terminate program\n";

    while (true) {
      renderOnce(kdt, name);
    }
  } else {
    // Run finite iterations.
    std::cout << "Rendering " << iterations << " iterations\n";

    for (int i = 0; i < iterations; ++i) {
      renderOnce(kdt, name);
    }
  }
}
