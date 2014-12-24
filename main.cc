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
  materials::Lambert    green(Vec(0.5f, 0.9f, 0.4f));
  materials::Lambert    white(Vec(1, 1, 1));
  materials::Lambert    red(Vec(0.9f, 0.4f, 0.5f));
  materials::Emitter    emit(Vec(4, 4, 4));
  materials::Dielectric dielectric(materials::Dielectric::IOR_GLASS);
  materials::Phong      glossy(100.0f);

  geoms::Sphere smallSphere(&dielectric, Vec(-6, -14, -16), 4.0f);
  geoms::Sphere bigSphere(&dielectric, Vec(10, -12, -24), 6.0f);
  geoms::Disc   bottom(&white, Vec(0, -18, -25), Vec(0, 1, 0), 100.0f);
  geoms::Disc   top(&white, Vec(0, 18, -25), Vec(0, -1, 0), 100.0f);
  geoms::Disc   back(&white, Vec(0, 0, -50), Vec(0, 0, 1), 100.0f);
  geoms::Disc   left(&red, Vec(-20, 0, -25), Vec(1, 0, 0), 100.0f);
  geoms::Disc   right(&green, Vec(20, 0, -25), Vec(-1, 0, 0), 100.0f);
  geoms::Sphere light(&emit, Vec(0, 46, -25), 30.0f);

  std::vector<Geom*> objs {
    &smallSphere, &bigSphere, &bottom, &top, &back, &left, &right, &light
  };

  // external model
  Mesh externalModel;
  externalModel.readPolyModel(
    &glossy,
    "assets/dragon.obj",
    Vec(-6, -18, -28),
    &objs
  );

  // Construct k-d tree acceleration structure.
  KDTree tree(&objs);
  tree.build();

  Camera cam(Ray(Vec(0, 0, 50), Vec(0, 0, -1)), w, h, math::PI_4);
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
