#include "camera.h"
#include <iostream>
#include <chrono>
#include "light.h"

using std::max;
using std::min;
namespace chrono = std::chrono;

Camera::Camera(
  const Transform& xform,
  const std::vector<const Geom*>& objs,
  long ww,
  long hh,
  float fov,
  float len,
  float fStop
) : accel(objs), focalLength(len),
    lensRadius((len / fStop) * 0.5f), // Diameter = focalLength / fStop.
    camToWorldXform(xform),
    masterRng(), rowSeeds(size_t(hh)), img(ww, hh), iters(0)
{
  // Calculate ray-tracing vectors.
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

  // Refine emitters so we can compute direct illumination.
  for (const Geom* g : objs) {
    if (g->light) {
      g->refine(emitters);
    }
  }
}

Camera::Camera(const Node& n)
  : Camera(math::rotationThenTranslation(
             n.getFloat("rotateAngle"),
             n.getVec("rotateAxis"),
             n.getVec("translate")
           ),
           n.getGeometryList("objects"),
           n.getInt("width"), n.getInt("height"),
           n.getFloat("fov"), n.getFloat("focalLength"),
           n.getFloat("fStop")) {}

void Camera::renderOnce(
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
    std::vector<RenderVertex> sharedEyePath;
    sharedEyePath.reserve(INITIAL_PATH_LENGTH);

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

        Vec L = trace(rng, Ray(eyeWorld, dir), sharedEyePath);
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
  std::string name,
  int iterations
) {
  if (iterations < 0) {
    // Run forever.
    std::cout << "Rendering infinitely, press Ctrl-c to terminate program\n";

    while (true) {
      renderOnce(name);
    }
  } else {
    // Run finite iterations.
    std::cout << "Rendering " << iterations << " iterations\n";

    for (int i = 0; i < iterations; ++i) {
      renderOnce(name);
    }
  }
}

Vec Camera::trace(
  Randomness& rng,
  Ray r,
  std::vector<RenderVertex>& sharedEyePath
) const {
  sharedEyePath.clear();
  randomWalk(rng, r, Vec(1, 1, 1), sharedEyePath);

  Vec L(0, 0, 0);
  bool didDirectIlluminate = false;

  for (const RenderVertex& vtx : sharedEyePath) {
    const Material* mat = vtx.geom->mat;
    const AreaLight* light = vtx.geom->light;

    // Check for lighting.
    if (light && !didDirectIlluminate) {
      // Accumulate emission normally if we did not direct-illuminate at the
      // last vertex. For any _n_, we can only count one _n_-length path per
      // sample trace.
      L += vtx.beta.cwiseProduct(light->emit(vtx));
    }

    // Direct-illuminate if possible.
    if (mat && mat->shouldDirectIlluminate()) {
#ifndef NO_DIRECT_ILLUM
      // Sample direct lighting and then continue path.
      L += vtx.beta.cwiseProduct(
        uniformSampleOneLight(rng, vtx)
      );
      didDirectIlluminate = true;
#else
      didDirectIlluminate = false;
#endif
    } else {
      didDirectIlluminate = false;
    }
  }

  L[0] = math::clamp(L[0], 0.0f, BIASED_RADIANCE_CLAMPING);
  L[1] = math::clamp(L[1], 0.0f, BIASED_RADIANCE_CLAMPING);
  L[2] = math::clamp(L[2], 0.0f, BIASED_RADIANCE_CLAMPING);

  return L;
}

void Camera::randomWalk(
  Randomness& rng,
  Ray r,
  Vec beta,
  std::vector<RenderVertex>& path
) const {
  for (int depth = 0; ; ++depth) {
    // Bounce ray and kill if nothing hit.
    RenderVertex isect(beta);
    if (accel.intersect(r, &isect)) {
      path.push_back(isect);
    } else {
      // End path in empty space.
      break;
    }

    // Check for scattering (reflection/transmission).
    if (isect.geom->mat) {
      isect.geom->mat->scatter(rng, isect, &r, &beta);
    } else {
      // Cannot continue path without a material.
      break;
    }

    // Do Russian Roulette if this path is "old".
    if (depth >= RUSSIAN_ROULETTE_DEPTH_1 || math::isNearlyZero(beta)) {
      float rv = rng.nextUnitFloat();

      float probLive;
      if (depth >= RUSSIAN_ROULETTE_DEPTH_2) {
        // More aggressive ray killing when ray is very old.
        probLive = math::clampedLerp(0.25f, 0.75f, math::luminance(beta));
      } else {
        // Less aggressive ray killing.
        probLive = math::clampedLerp(0.25f, 1.00f, math::luminance(beta));
      }

      if (rv < probLive) {
        // The ray lives (more energy = more likely to live).
        // Increase its energy to balance out probabilities.
        beta = beta / probLive;
      } else {
        // The ray dies.
        break;
      }
    }
  }
}

Vec Camera::uniformSampleOneLight(
  Randomness& rng,
  const Intersection& isect
) const {
  size_t numLights = emitters.size();
  if (numLights == 0) {
    return Vec(0, 0, 0);
  }

  size_t lightIdx = size_t(floorf(rng.nextUnitFloat() * numLights));
  const Geom* emitter = emitters[min(lightIdx, numLights - 1)];
  const AreaLight* areaLight = emitter->light;

  // P[this light] = 1 / numLights, so 1 / P[this light] = numLights.
  return float(numLights) * areaLight->directIlluminate(
    rng, isect, emitter, &accel
  );
}
