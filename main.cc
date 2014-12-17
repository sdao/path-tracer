#include <vector>
#include <memory>
#include <iostream>
#include "materials/all.h"
#include "geoms/all.h"
#include "camera.h"
#include "kdtree.h"

[[noreturn]] void render(size_t w, size_t h, std::string name);

void render(size_t w, size_t h, std::string name) {
  materialptr green = materials::diffuse::make(vec(0.5, 0.9, 0.4));
  materialptr white = materials::diffuse::make(vec(1, 1, 1));
  materialptr blue = materials::diffuse::make(vec(0.5, 0.6, 1));
  materialptr emit = materials::emitter::make(vec(4, 4, 4));
  materialptr fresnel = materials::fresnel::make();
  materialptr spec = materials::glossy::make(0.25f);

  // Construct k-d tree acceleration structure.
  kdtree tree;

  std::vector<geomptr>& objs = tree.objs;
  // spheres
  objs.push_back(
    geoms::sphere::make(spec, vec(-8, -10, -36), 8.0f)
  );
  objs.push_back(
    geoms::sphere::make(fresnel, vec(10, -10, -24), 8.0f)
  );
  // bottom
  objs.push_back(
    geoms::disc::make(white, vec(0, -18, -25), vec(0, 1, 0), 100.0f)
  );
  // top
  objs.push_back(
    geoms::disc::make(white, vec(0, 18, -25), vec(0, -1, 0), 100.0f)
  );
  // back
  objs.push_back(
    geoms::disc::make(white, vec(0, 0, -50), vec(0, 0, 1), 100.0f)
  );
  // left
  objs.push_back(
    geoms::disc::make(blue, vec(-20, 0, -25), vec(1, 0, 0), 100.0f)
  );
  // right
  objs.push_back(
    geoms::disc::make(green, vec(20, 0, -25), vec(-1, 0, 0), 100.0f)
  );
  // light
  objs.push_back(
    geoms::sphere::make(emit, vec(0, 46, -25), 30.0f)
  );

  tree.build();
  std::cout << tree << "\n";

  camera cam(ray(vec(0, 0, 50), vec(0, 0, -1)), w, h, float(M_PI_4));
  cam.renderInfinite(tree, name);

  /* Will not return. */
}

int main() {
  render(512, 384, "/Users/Steve/Desktop/sample.exr");

  /* Will not return. */
}
