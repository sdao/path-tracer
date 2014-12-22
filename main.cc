#include <vector>
#include <iostream>
#include <sstream>
#include "materials/all.h"
#include "geoms/all.h"
#include "camera.h"
#include "kdtree.h"
#include "mesh.h"

void render(size_t w, size_t h, int iterations);

void render(size_t w, size_t h, int iterations) {
  materials::diffuse green(vec(0.5f, 0.9f, 0.4f));
  materials::diffuse white(vec(1, 1, 1));
  materials::diffuse blue(vec(0.5f, 0.6f, 1));
  materials::emitter emit(vec(4, 4, 4));
  materials::fresnel fresnel(materials::fresnel::IOR_GLASS);
  materials::phong   glossy(100.0f);

  geoms::sphere smallSphere(&fresnel, vec(-6, -14, -16), 4.0f);
  geoms::sphere bigSphere(&fresnel, vec(10, -12, -24), 6.0f);
  geoms::disc   bottom(&white, vec(0, -18, -25), vec(0, 1, 0), 100.0f);
  geoms::disc   top(&white, vec(0, 18, -25), vec(0, -1, 0), 100.0f);
  geoms::disc   back(&white, vec(0, 0, -50), vec(0, 0, 1), 100.0f);
  geoms::disc   left(&blue, vec(-20, 0, -25), vec(1, 0, 0), 100.0f);
  geoms::disc   right(&green, vec(20, 0, -25), vec(-1, 0, 0), 100.0f);
  geoms::sphere light(&emit, vec(0, 46, -25), 30.0f);

  std::vector<geom*> objs {
    &smallSphere, &bigSphere, &bottom, &top, &back, &left, &right, &light
  };

  // external model
  mesh externalModel;
  externalModel.readPolyModel(
    &glossy,
    "assets/dragon.obj",
    vec(-6, -18, -28),
    &objs
  );

  // Construct k-d tree acceleration structure.
  kdtree tree(&objs);
  tree.build();

  camera cam(ray(vec(0, 0, 50), vec(0, 0, -1)), w, h, math::PI_4);
  cam.renderMultiple(tree, "output.exr", iterations);
}

int main(int argc, char* argv[]) {
  int iterations = -1;

  if (argc >= 2) {
    std::istringstream itersArg(argv[1]);
    itersArg >> iterations;
  }

  render(512, 384, iterations);
}
