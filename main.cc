#include <vector>
#include <iostream>
#include "materials/all.h"
#include "geoms/all.h"
#include "camera.h"
#include "kdtree.h"
#include "mesh.h"

[[noreturn]] void render(size_t w, size_t h, std::string name);

void render(size_t w, size_t h, std::string name) {
  material* green = new materials::diffuse(vec(0.5, 0.9, 0.4));
  material* white = new materials::diffuse(vec(1, 1, 1));
  material* blue = new materials::diffuse(vec(0.5, 0.6, 1));
  material* emit = new materials::emitter(vec(4, 4, 4));
  //material* fresnel = new materials::fresnel();
  material* spec = new materials::glossy(0.25f);
  //material* normaltest = new materials::normaltest();

  std::vector<geom*> objs;
  // spheres
  //objs.push_back(
  //  new geoms::sphere(spec, vec(-8, -10, -36), 8.0f)
  //);
  //objs.push_back(
  //  new geoms::sphere(fresnel, vec(10, -10, -20), 8.0f)
  //);
  // bottom
  objs.push_back(
    new geoms::disc(white, vec(0, -18, -25), vec(0, 1, 0), 100.0f)
  );
  // top
  objs.push_back(
    new geoms::disc(white, vec(0, 18, -25), vec(0, -1, 0), 100.0f)
  );
  // back
  objs.push_back(
    new geoms::disc(white, vec(0, 0, -50), vec(0, 0, 1), 100.0f)
  );
  // left
  objs.push_back(
    new geoms::disc(blue, vec(-20, 0, -25), vec(1, 0, 0), 100.0f)
  );
  // right
  objs.push_back(
    new geoms::disc(green, vec(20, 0, -25), vec(-1, 0, 0), 100.0f)
  );
  // light
  objs.push_back(
    new geoms::sphere(emit, vec(0, 46, -25), 30.0f)
  );
  
  // external model
  mesh externalModel;
  externalModel.readPolyModel(
    spec,
    "assets/cone20.obj",
    vec(-8, -18, -32),
    &objs
  );

  // Construct k-d tree acceleration structure.
  kdtree tree(&objs);
  tree.build();

  camera cam(ray(vec(0, 0, 50), vec(0, 0, -1)), w, h, float(M_PI_4));
  cam.renderInfinite(tree, name);

  /* Will not return. We do not have to clean up objs or materials! */
}

int main() {
  render(512, 384, "output.exr");

  /* Will not return. */
}
