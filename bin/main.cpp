#include <cstdlib>
#include <format>
#include <iostream>
#include <string>
#include <vector>

#include "Platform.h"

namespace config = malachi::config;

auto main(int argc, char *argv[]) -> int {
  const std::vector<std::string> args(argv, argv + argc);

  if (args.size() != 2) {
    std::cerr << std::format("Usage: {} <filename>\n", args[0]);
    return EXIT_FAILURE;
  }

  auto platform = config::get_platform();

  std::cout << std::format("Platform: {}\n", to_string(platform));

  return EXIT_SUCCESS;
}
