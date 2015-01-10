#include <iostream>
#include <boost/program_options.hpp>
#include "scene.h"
#include "camera.h"
#include "debug.h"

int main(int argc, char* argv[]) {
  using namespace boost::program_options;

  try {
    // Parse command-line args using boost::program_options.
    options_description desc("Allowed options");
    desc.add_options()
      ("help",
        "produce help message")
      ("input", value<std::string>()->required(),
        "JSON scene file input")
      ("output", value<std::string>()->default_value("output.exr"),
        "EXR output path")
      ("iterations", value<int>()->default_value(-1),
        "path-tracing iterations, if < 0 then will run forever");

    positional_options_description pd;
    pd.add("input", 1).add("output", 1).add("iterations", 1);

    variables_map vars;
    store(
      command_line_parser(argc, argv).options(desc).positional(pd).run(), vars
    );

    // Print help message if requested by user.
    if (vars.count("help")) {
      std::cout << desc;
      return 0;
    }

    // Raise errors after checking the help flag.
    notify(vars);

    // Load scene and set up rendering.
    std::string input = vars["input"].as<std::string>();
    std::string output = vars["output"].as<std::string>();
    int iterations = vars["iterations"].as<int>();

    Scene scene(input);
    scene.defaultCamera()->renderMultiple(output, iterations);
  } catch (std::exception& e) {
    debug::printNestedException(e);
    return 42;
  }

  return 0;
}
