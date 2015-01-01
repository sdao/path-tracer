#include <vector>
#include <iostream>
#include <sstream>
#include "materials/all.h"
#include "geoms/all.h"
#include "light.h"
#include "camera.h"
#include "kdtree.h"

void renderCornellBox(long w, long h, int iterations);
void renderDaylightScene(long w, long h, int iterations);
void renderInversionTest(long w, long h, int iterations);

void renderCornellBox(long w, long h, int iterations) {
  AreaLight areaLight(Vec(4, 4, 4));

  materials::Lambert    green(Vec(0.5f, 1.0f, 0.5f));
  materials::Lambert    white(Vec(1, 1, 1));
  materials::Lambert    red(Vec(1.0f, 0.5f, 0.5f));
  materials::Dielectric dielectric(materials::Dielectric::IOR_GLASS);
  materials::Phong      glossy(100.0f);

  geoms::Sphere smallSphere(Vec(-6, -14, -16), 4.0f, &dielectric);
  geoms::Sphere bigSphere(Vec(10, -12, -22), 6.0f, &dielectric);
  geoms::Disc   bottom(Vec(0, -18, -25), Vec(0, 1, 0), 100.0f, 0.0f, &white);
  geoms::Disc   top(Vec(0, 18, -25), Vec(0, -1, 0), 100.0f, 10.0f, &white);
  geoms::Disc   back(Vec(0, 0, -50), Vec(0, 0, 1), 100.0f, 0.0f, &white);
  geoms::Disc   left(Vec(-18, 0, -25), Vec(1, 0, 0), 100.0f, 0.0f, &red);
  geoms::Disc   right(Vec(18, 0, -25), Vec(-1, 0, 0), 100.0f, 0.0f, &green);
  geoms::Disc   lightSource(
    Vec(0.0f, 18.5f, -25.0f),
    Vec(0, -1, 0),
    12.0f, 0.0f,
    nullptr,
    &areaLight
  );
  geoms::Mesh   dragon(Vec(-6, -18, -28), &glossy);
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
    45.0f // Focus on small sphere (its origin is ~48 away).
  );
  cam.renderMultiple(tree, "output.exr", iterations);
}

void renderDaylightScene(long w, long h, int iterations) {
  AreaLight areaLight(Vec(1, 1, 1));

  materials::Phong      glossy(5.0f, Vec(1.0f, 0.9f, 0.8f));
  materials::Lambert    white(Vec(1, 1, 1));
  materials::Dielectric blueDielectric(
    materials::Dielectric::IOR_GLASS,
    Vec(0.6f, 0.8f, 1.0f)
  );
  materials::Dielectric redDielectric(
    materials::Dielectric::IOR_GLASS,
    Vec(1.0f, 0.6f, 0.8f)
  );
  materials::Dielectric greenDielectric(
    materials::Dielectric::IOR_GLASS,
    Vec(0.6f, 1.0f, 0.8f)
  );

  geoms::Sphere   smallSphere1(Vec(-6, -14, -26), 4.0f, &redDielectric);
  geoms::Sphere   smallSphere2(Vec(14, -14, -30), 4.0f, &redDielectric);
  geoms::Sphere   smallSphere3(Vec(4, -14, -26), 4.0f, &greenDielectric);
  geoms::Sphere   smallSphere4(Vec(0, -15, -22), 3.0f, &redDielectric);
  geoms::Sphere   bigSphere(Vec(8, -12, -20), 6.0f, &blueDielectric);
  geoms::Disc     bottom(Vec(0, -18, -25), Vec(0, 1, 0), 10000.0f, 0.0f, &white);
  geoms::Sphere   giant(Vec(0, -18, -25), 10000.0f, nullptr, &areaLight);
  geoms::Inverted lightSource(&giant);
  geoms::Mesh     dragon(Vec(-10, -18, -35), &glossy);
  dragon.readPolyModel("assets/dragon100k.obj");

  std::vector<const Geom*> objs {
    &smallSphere1, &smallSphere2, &smallSphere3, &smallSphere4, &bigSphere,
    &bottom, &lightSource, &dragon
  };

  // Construct k-d tree acceleration structure.
  KDTree tree(objs);
  tree.build();

  Camera cam(
    math::angleAxisRotation(-math::PI_4, Vec(1, 0, 0))
      * math::translation(0, 10, -10),
    w, h,
    math::PI_2,
    15.0f, // Aim at the red sphere on the left.
    8.0f
  );
  cam.renderMultiple(tree, "output.exr", iterations);
}

void renderInversionTest(long w, long h, int iterations) {
  AreaLight areaLight(Vec(1, 1, 1));

  materials::Lambert white(Vec(1, 1, 1));
  materials::Phong   glossy(100.0f);

  geoms::Sphere smallSphere(Vec(-6, -14, -16), 4.0f, nullptr, &areaLight);
  geoms::Inverted smallSphereInv(&smallSphere);

  geoms::Sphere bigSphere(Vec(10, -12, -24), 6.0f, nullptr, &areaLight);
  geoms::Inverted bigSphereInv1(&bigSphere);
  geoms::Inverted bigSphereInv2(&bigSphereInv1);

  geoms::Disc   bottom(Vec(0, -18, -25), Vec(0, 1, 0), 100.0f, 0.0f, &white);
  geoms::Disc   lightSource(
    Vec(0.0f, 10.0f, -25.0f),
    Vec(0, -1, 0),
    15.0f, 0.0f,
    nullptr,
    &areaLight
  );

  std::vector<const Geom*> objs {
    &smallSphereInv, &bigSphereInv2, &bottom, &lightSource
  };

  // Construct k-d tree acceleration structure.
  KDTree tree(objs);
  tree.build();

  Camera cam(math::translation(0, 0, 32), w, h, math::PI_4, 50.0f, 1024.0f);
  cam.renderMultiple(tree, "output.exr", iterations);
}

int main(int argc, char* argv[]) {
  int iterations = -1;

  if (argc >= 2) {
    std::istringstream itersArg(argv[1]);
    itersArg >> iterations;
  }

  renderCornellBox(512, 384, iterations);
}
