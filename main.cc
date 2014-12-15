#include <vector>
#include <memory>
#include "material.h"
#include "sphere.h"
#include "plane.h"
#include "camera.h"

void render(int w, int h) {
  materialptr green = std::make_shared<idealdiffuse>(vec(0.5, 0.9, 0.4));
  materialptr white = std::make_shared<idealdiffuse>(vec(1, 1, 1));
  materialptr blue = std::make_shared<idealdiffuse>(vec(0.5, 0.6, 1));
  materialptr emit = std::make_shared<idealemitter>(vec(4, 4, 4));
  materialptr fresnel = std::make_shared<fresnelrefract>();
  materialptr spec = std::make_shared<idealspecular>();

  std::vector<geomptr> objs;
  objs.push_back(std::make_shared<sphere>(vec(-8, -10, -36), 8.0f, spec));
  objs.push_back(std::make_shared<sphere>(vec(8, -10, -24), 8.0f, fresnel));
  objs.push_back(std::make_shared<plane>(vec(0, -18, 0), vec(0, 1, 0), white)); // bottom
  objs.push_back(std::make_shared<plane>(vec(0, 18, 0), vec(0, -1, 0), white)); // top
  objs.push_back(std::make_shared<plane>(vec(0, 0, -50), vec(0, 0, 1), white)); // back
  objs.push_back(std::make_shared<plane>(vec(-20, 0, 0), vec(1, 0, 0), blue)); // left
  objs.push_back(std::make_shared<plane>(vec(20, 0, 0), vec(-1, 0, 0), green)); // right
  objs.push_back(std::make_shared<sphere>(vec(0, 46, -30), 30.0f, emit)); // light

  camera cam(ray(vec(0, 0, 50), vec(0, 0, -100)), w, h, M_PI / 4.0f);
  cam.renderInfinite(objs, "/Users/Steve/Desktop/sample.exr");
}

int main() {
  int w = 512;
  int h = 384;

  render(512, 384);

  return 0;
}
