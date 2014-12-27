#include "camera.h"
#include <iostream>
#include <chrono>

namespace chrono = std::chrono;

Camera::Camera(Ray e, size_t ww, size_t hh, float ff)
  : eye(e.origin, e.direction.normalized()), fovx2(0.5f * ff), masterRng(),
    rowSeeds(hh), colors(hh), weights(hh), exrData(long(hh), long(ww)),
    w(ww), h(hh), iters(0)
{
  // Size of the image plane projected into world space
  // using the given fovx and cam focal length.
  float scaleRight = 2.0f * eye.direction.norm() * tanf(fovx2);
  float scaleUp = scaleRight * (float(h) / float(w));

  // Corresponding vectors.
  up = -Vec(0, 1, 0) * scaleUp; // Flip the y-axis for image output!
  right = -eye.direction.cross(up).normalized() * scaleRight;

  // Image corner ray in world space.
  cornerRay = eye.direction - (0.5f * up) - (0.5f * right);

  // Prepare accumulated color and weight buffers.
  for (size_t y = 0; y < h; ++y) {
    colors[y] = std::vector<DoubleVec>(w);
    weights[y] = std::vector<double>(w);
    for (size_t x = 0; x < w; ++x) {
      colors[y][x] = DoubleVec(0, 0, 0);
      weights[y][x] = 0.0;
    }
  }
}

void Camera::renderOnce(const KDTree& kdt, std::string name) {
  iters++;
  std::cout << "Iteration " << iters;
  chrono::steady_clock::time_point startTime = chrono::steady_clock::now();

  for (size_t y = 0; y < h; ++y) {
    rowSeeds[y] = masterRng.nextUnsigned();
  }

  tbb::parallel_for(size_t(0), size_t(h), [&](size_t y) {
  //for (size_t y = 0; y < h; ++y) {
    Randomness rng(rowSeeds[y]);

    for (size_t x = 0; x < w; ++x) {
      DoubleVec& pxColor = colors[y][x];
      double& pxWeight = weights[y][x];

      // Begin pixel sampling loop.
      for (int samps = 0; samps < SAMPLES_PER_PIXEL; ++samps) {
        float offsetY = rng.nextFloat(-0.5f, 0.5f);
        float offsetX = rng.nextFloat(-0.5f, 0.5f);
        float fracY = (float(y) + offsetY) / (float(h) - 1.0f);
        float fracX = (float(x) + offsetX) / (float(w) - 1.0f);

        LightRay r(
          eye.origin,
          (cornerRay + (up * fracY) + (right * fracX)).normalized()
        );

        int depth = 0;
        while (!r.isZeroLength()) {
          depth++;

          // Do Russian Roulette if this path is "old".
          if (depth > RUSSIAN_ROULETTE_DEPTH_1 || r.isBlack()) {
            float rv = rng.nextUnitFloat();

            float probLive;
            if (depth > RUSSIAN_ROULETTE_DEPTH_2) {
              // More aggressive ray killing when ray is very old.
              probLive = math::clampedLerp(0.0f, 0.5f, r.energy());
            } else {
              // Less aggressive ray killing.
              probLive = math::clamp(r.energy());
            }

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
          Intersection isect;
          Geom* g = kdt.intersect(r, &isect);

          if (g) {
            r = g->mat->propagate(r, isect, rng);
          } else {
            r.kill();
            break;
          }
        } // End pixel sampling loop.

        // Filter value.
        double smpWeight = math::mitchellFilter(offsetX, offsetY);

        // Black if ray died; fill in color otherwise.
        pxColor += smpWeight * DoubleVec(r.color.x(), r.color.y(), r.color.z());
        pxWeight += smpWeight;
      }
    } // end of x-for loop
  //} // end of y-for loop
  });

  math::reconstructImage(w, h, colors, weights, exrData);
  Imf::RgbaOutputFile file(name.c_str(), int(w), int(h), Imf::WRITE_RGBA);
  file.setFrameBuffer(&exrData[0][0], 1, w);
  file.writePixels(int(h));

  chrono::steady_clock::time_point endTime = chrono::steady_clock::now();
  chrono::duration<double> runTime =
    chrono::duration_cast<chrono::duration<double>>(endTime - startTime);
  std::cout << " [" << runTime.count() << " seconds]\n";
}

void Camera::renderMultiple(
  const KDTree& kdt,
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
