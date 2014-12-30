#include <vector>
#include <iostream>
#include <sstream>
#include "materials/all.h"
#include "geoms/all.h"
#include "light.h"
#include "camera.h"
#include "kdtree.h"
#include "mesh.h"

void render(long w, long h, int iterations);

void render(long w, long h, int iterations) {
  materials::Lambert    green(Vec(0.5f, 1.0f, 0.5f));
  materials::Lambert    white(Vec(1, 1, 1));
  materials::Lambert    red(Vec(1.0f, 0.5f, 0.5f));
  materials::Dielectric dielectric(materials::Dielectric::IOR_GLASS);
  materials::Phong      glossy(100.0f);

  AreaLight areaLight(Vec(4, 4, 4));

  geoms::Sphere smallSphere(Vec(-6, -14, -16), 4.0f, &dielectric);
  geoms::Sphere bigSphere(Vec(10, -12, -24), 6.0f, &dielectric);
  geoms::Disc   bottom(Vec(0, -18, -25), Vec(0, 1, 0), 100.0f, 0.0f, &white);
  geoms::Disc   top(Vec(0, 18, -25), Vec(0, -1, 0), 100.0f, 10.0f, &white);
  geoms::Disc   back(Vec(0, 0, -50), Vec(0, 0, 1), 100.0f, 0.0f, &white);
  geoms::Disc   left(Vec(-20, 0, -25), Vec(1, 0, 0), 100.0f, 0.0f, &red);
  geoms::Disc   right(Vec(20, 0, -25), Vec(-1, 0, 0), 100.0f, 0.0f, &green);
  geoms::Disc   lightSource(
    Vec(0.0f, 18.5f, -25.0f),
    Vec(0, -1, 0),
    12.0f, 0.0f,
    nullptr,
    &areaLight
  );

  std::vector<Geom*> objs {
    &smallSphere, &bigSphere, &bottom, &top, &back, &left, &right, &lightSource
  };

  std::vector<Geom*> lights {
    &lightSource
  };

  // external model
  Mesh externalModel;
  externalModel.readPolyModel(
    "assets/dragon.obj",
    Vec(-6, -18, -28),
    &glossy,
    &objs
  );

  // Construct k-d tree acceleration structure.
  KDTree tree(&objs);
  tree.build();

  Camera cam(Ray(Vec(0, 0, 50), Vec(0, 0, -1)), w, h, math::PI_4);
  cam.renderMultiple(tree, lights, "output.exr", iterations);
}

int main(int argc, char* argv[]) {
  int iterations = -1;

  if (argc >= 2) {
    std::istringstream itersArg(argv[1]);
    itersArg >> iterations;
  }

  render(512, 384, iterations);
}
