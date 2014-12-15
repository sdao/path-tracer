#include <vector>
#include <memory>
#include "materials/all.h"
#include "geoms/all.h"
#include "camera.h"

void render(int w, int h) {
  materialptr green = materials::diffuse::make(vec(0.5, 0.9, 0.4));
  materialptr white = materials::diffuse::make(vec(1, 1, 1));
  materialptr blue = materials::diffuse::make(vec(0.5, 0.6, 1));
  materialptr emit = materials::emitter::make(vec(4, 4, 4));
  materialptr fresnel = materials::fresnel::make();
  materialptr spec = materials::glossy::make(0.25f);

  std::vector<geomptr> objs;
  objs.push_back(geoms::sphere::make(vec(-8, -10, -36), 8.0f, spec));
  objs.push_back(geoms::sphere::make(vec(10, -10, -24), 8.0f, fresnel));
  objs.push_back(geoms::plane::make(vec(0, -18, 0), vec(0, 1, 0), white)); // bottom
  objs.push_back(geoms::plane::make(vec(0, 18, 0), vec(0, -1, 0), white)); // top
  objs.push_back(geoms::plane::make(vec(0, 0, -50), vec(0, 0, 1), white)); // back
  objs.push_back(geoms::plane::make(vec(-20, 0, 0), vec(1, 0, 0), blue)); // left
  objs.push_back(geoms::plane::make(vec(20, 0, 0), vec(-1, 0, 0), green)); // right
  objs.push_back(geoms::sphere::make(vec(0, 46, -25), 30.0f, emit)); // light

  camera cam(ray(vec(0, 0, 50), vec(0, 0, -100)), w, h, M_PI / 4.0f);
  cam.renderInfinite(objs, "/Users/Steve/Desktop/sample.exr");
}

int main() {
  int w = 512;
  int h = 384;

  render(512, 384);

  return 0;
}
