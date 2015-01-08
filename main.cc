#include <vector>
#include <iostream>
#include <sstream>
#include "materials/all.h"
#include "geoms/all.h"
#include "light.h"
#include "camera.h"
#include "kdtree.h"

void renderCornellBox(long w, long h, int iterations);
void renderSpheresScene(long w, long h, int iterations);

void renderCornellBox(long w, long h, int iterations) {
  AreaLight areaLight(Vec(4, 4, 4));

  materials::Lambert    green(Vec(0.5f, 1.0f, 0.5f));
  materials::Lambert    white(Vec(1, 1, 1));
  materials::Lambert    red(Vec(1.0f, 0.5f, 0.5f));
  materials::Dielectric dielectric(materials::Dielectric::IOR_GLASS);
  materials::Phong      glossy(100.0f);

  geoms::Sphere smallSphere(Vec(-2, -14, -16), 4.0f, &dielectric);
  geoms::Sphere bigSphere(Vec(10, -12, -22), 6.0f, &dielectric);
  geoms::Disc   bottom(Vec(0, -18, -25), Vec(0, 1, 0), 100.0f, 0.0f, &white);
  geoms::Disc   top(Vec(0, 18, -25), Vec(0, -1, 0), 100.0f, 10.0f, &white);
  geoms::Disc   back(Vec(0, 0, -50), Vec(0, 0, 1), 100.0f, 0.0f, &white);
  geoms::Disc   left(Vec(-22, 0, -25), Vec(1, 0, 0), 100.0f, 0.0f, &red);
  geoms::Disc   right(Vec(22, 0, -25), Vec(-1, 0, 0), 100.0f, 0.0f, &green);
  geoms::Disc   lightSource(
    Vec(0.0f, 18.5f, -25.0f),
    Vec(0, -1, 0),
    12.0f, 0.0f,
    nullptr,
    &areaLight
  );
  geoms::Mesh   dragon(Vec(-8, -18, -28), &glossy);
  dragon.readPolyModel("assets/dragon100k.obj");

  std::vector<const Geom*> objs {
    &smallSphere, &bigSphere, &bottom, &top, &back, &left, &right, &lightSource,
    &dragon
  };

  // Construct k-d tree acceleration structure.
  KDTree tree(objs);
  tree.build();

  Camera cam(
    math::translation(0, 0, 32),
    w, h,
    math::PI_4,
    48.0f,
    32.0f
  );
  cam.renderMultiple(tree, "output.exr", iterations);
}

void renderSpheresScene(long w, long h, int iterations) {
  AreaLight areaLight(Vec(10, 10, 10));
  AreaLight skyLight(Vec(0.3f, 0.4f, 0.5f));

  materials::Lambert    green(Vec(0.5f, 1.0f, 0.5f));
  materials::Lambert    ground(Vec(0.4f, 0.3f, 0.2f));
  materials::Lambert    skyBlue(Vec(0.4f, 0.5f, 0.6f));
  materials::Lambert    red(Vec(1.0f, 0.5f, 0.5f));
  materials::Dielectric dielectric(materials::Dielectric::IOR_GLASS);
  materials::Phong      glossy(100.0f);

  geoms::Sphere   worldSphere(Vec(0, 0, 0), 2000.0f, &skyBlue, &skyLight);
  geoms::Inverted worldSphereInv(&worldSphere);
  geoms::Sphere   lightSource(Vec(0, 2500, 0), 1000, nullptr, &areaLight);
  geoms::Disc     bottom(Vec(0, -20, 0), Vec(0, 1, 0), 2001.0f, 0.0f, &ground);

  std::vector<const Geom*> objs {
    &bottom, &worldSphereInv, &lightSource
  };

  // Construct diagonal line of spheres.
  std::vector<geoms::Sphere> spheres;
  for (int i = 0; i < 10; ++i) {
    geoms::Sphere bigSphere(
      Vec(16 - 14.0f * float(i), -12, -22 - 18.0f * float(i)),
      8.0f,
      &dielectric
    );
    spheres.push_back(bigSphere);
  }
  for (const geoms::Sphere& s : spheres) {
    objs.push_back(&s);
  }

  // Construct k-d tree acceleration structure.
  KDTree tree(objs);
  tree.build();

  Camera cam(
    math::angleAxisRotation(-math::PI / 12.0f, Vec(1, 0, 0))
      * math::translation(0, 0, 32),
    w, h,
    math::PI_4,
    88.0f,
    8.0f
  );
  cam.renderMultiple(tree, "output.exr", iterations);
}

int main(int argc, char* argv[]) {
  int iterations = -1;

  if (argc >= 2) {
    std::istringstream itersArg(argv[1]);
    itersArg >> iterations;
  }

  renderSpheresScene(512, 384, iterations);
}
