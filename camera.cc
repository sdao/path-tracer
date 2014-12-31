#include "camera.h"
#include <iostream>
#include <chrono>
#include "light.h"

using std::max;
using std::min;
namespace chrono = std::chrono;

Camera::Camera(
  Transform xform,
  long ww,
  long hh,
  float fov,
  float len,
  float stop
) : focalLength(len), lensRadius((len / stop) * 0.5f), // Diameter = len / stop.
    camToWorldXform(xform),
    masterRng(), rowSeeds(size_t(hh)), img(ww, hh), iters(0)
{
  float halfFocalPlaneUp;
  float halfFocalPlaneRight;

  if (img.w > img.h) {
    halfFocalPlaneUp = focalLength * tanf(0.5f * fov);
    halfFocalPlaneRight = halfFocalPlaneUp * float(img.w) / float(img.h);
  } else {
    halfFocalPlaneRight = focalLength * tanf(0.5f * fov);
    halfFocalPlaneUp = halfFocalPlaneRight * float(img.h) / float(img.w);
  }

  focalPlaneUp = -2.0f * halfFocalPlaneUp;
  focalPlaneRight = 2.0f * halfFocalPlaneRight;
  focalPlaneOrigin = Vec(-halfFocalPlaneRight, halfFocalPlaneUp, -focalLength);
}

void Camera::renderOnce(
  const KDTree& kdt,
  const std::vector<Geom*>& lights,
  std::string name
) {
  // Increment iteration count and begin timer.
  iters++;
  std::cout << "Iteration " << iters;
  chrono::steady_clock::time_point startTime = chrono::steady_clock::now();

  // Seed the per-row RNGs.
  for (long y = 0; y < img.h; ++y) {
    rowSeeds[size_t(y)] = masterRng.nextUnsigned();
  }

  // Trace paths in parallel using TBB.
  tbb::parallel_for(long(0), img.h, [&](long y) {
    Randomness rng(rowSeeds[size_t(y)]);
    for (long x = 0; x < img.w; ++x) {
      for (long samp = 0; samp < img.samplesPerPixel; ++samp) {
        float offsetY = rng.nextFloat(-img.filterWidth, img.filterWidth);
        float offsetX = rng.nextFloat(-img.filterWidth, img.filterWidth);

        float posY = float(y) + offsetY;
        float posX = float(x) + offsetX;

        float fracY = posY / (float(img.h) - 1.0f);
        float fracX = posX / (float(img.w) - 1.0f);

        // Implement depth of field by jittering the eye.
        Vec offset(focalPlaneRight * fracX, focalPlaneUp * fracY, 0);
        Vec lookAt = focalPlaneOrigin + offset;

        Vec eye(0, 0, 0);
        math::areaSampleDisk(rng, &eye[0], &eye[1]);
        eye = eye * lensRadius;

        Vec eyeWorld = camToWorldXform * eye;
        Vec lookAtWorld = camToWorldXform * lookAt;
        Vec dir = (lookAtWorld - eyeWorld).normalized();
      
        Vec L = trace(LightRay(eyeWorld, dir), rng, kdt, lights);
        img.setSample(x, y, posX, posY, samp, L);
      }
    }
  });

  // Process and write the output file at the end of this iteration.
  img.commitSamples();
  img.writeToEXR(name);

  // End timer.
  chrono::steady_clock::time_point endTime = chrono::steady_clock::now();
  chrono::duration<float> runTime =
    chrono::duration_cast<chrono::duration<float>>(endTime - startTime);
  std::cout << " [" << runTime.count() << " seconds]\n";
}

void Camera::renderMultiple(
  const KDTree& kdt,
  const std::vector<Geom*>& lights,
  std::string name,
  int iterations
) {
  if (iterations < 0) {
    // Run forever.
    std::cout << "Rendering infinitely, press Ctrl-c to terminate program\n";

    while (true) {
      renderOnce(kdt, lights, name);
    }
  } else {
    // Run finite iterations.
    std::cout << "Rendering " << iterations << " iterations\n";

    for (int i = 0; i < iterations; ++i) {
      renderOnce(kdt, lights, name);
    }
  }
}

Vec Camera::trace(
  LightRay r,
  Randomness& rng,
  const KDTree& kdt,
  const std::vector<Geom*>& lights
) const {
  Vec L(0, 0, 0);
  bool didDirectIlluminate = false;

  for (int depth = 0; ; ++depth) {
    // Do Russian Roulette if this path is "old".
    if (depth >= RUSSIAN_ROULETTE_DEPTH_1 || r.isBlack()) {
      float rv = rng.nextUnitFloat();

      float probLive;
      if (depth >= RUSSIAN_ROULETTE_DEPTH_2) {
        // More aggressive ray killing when ray is very old.
        probLive = math::clampedLerp(0.05f, 0.75f, r.energy());
      } else {
        // Less aggressive ray killing.
        probLive = math::clampedLerp(0.25f, 1.00f, r.energy());
      }

      if (rv < probLive) {
        // The ray lives (more energy = more likely to live).
        // Increase its energy to balance out probabilities.
        r.color = r.color / probLive;
      } else {
        // The ray dies.
        break;
      }
    }

    // Bounce ray and kill if nothing hit.
    Intersection isect;
    const Geom* g = kdt.intersect(r, &isect);
    if (!g) {
      // End path in empty space.
      break;
    }

    // Check for lighting.
    if (g->light && !didDirectIlluminate) {
      // Accumulate emission normally.
      L += r.color.cwiseProduct(g->light->emit(isect, r.direction));
    } else if (g->light && didDirectIlluminate) {
      // Skip emission accumulation because it was accumulated already
      // in a direct lighting calculation. We don't want to double-count.
    }

    // Check for scattering (reflection/transmission).
    if (!g->mat) {
      // Cannot continue path without a material.
      break;
    } else if (g->mat && !g->mat->shouldDirectIlluminate()) {
      // Continue path normally.
      r = g->mat->scatter(rng, r, isect);
      didDirectIlluminate = false;
    } else if (g->mat && g->mat->shouldDirectIlluminate()) {
      // Sample direct lighting and then continue path.
      L += r.color.cwiseProduct(
        uniformSampleOneLight(rng, r, isect, g->mat, kdt, lights)
      );
      r = g->mat->scatter(rng, r, isect);
      didDirectIlluminate = true;
    }
  }

  return L;
}

Vec Camera::uniformSampleOneLight(
  Randomness& rng,
  const LightRay& incoming,
  const Intersection& isect,
  const Material* mat,
  const KDTree& kdt,
  const std::vector<Geom*>& lights
) const {
  size_t numLights = lights.size();
  if (numLights == 0) {
    return Vec(0, 0, 0);
  }

  size_t lightIdx = size_t(floorf(rng.nextUnitFloat() * numLights));
  const Geom* emissionObj = lights[min(lightIdx, numLights - 1)];
  const AreaLight* areaLight = emissionObj->light;

  // P[this light] = 1 / numLights, so 1 / P[this light] = numLights.
  return float(numLights)
    * areaLight->directIlluminate(rng, incoming, isect, mat, emissionObj, kdt);
}
