#include <cstdlib>
#include <format>
#include <iostream>
#include <string>
#include <vector>

#include "Config.h"
#include "Platform.h"

namespace config = malachi::config;

using config::Config;
using config::ConfigBuilder;

auto main(int argc, char *argv[]) -> int
{
    const std::vector<std::string> args(argv, argv + argc);

    if (args.size() != 2) {
        std::cerr << std::format("Usage: {} <filename>\n", args[0]);
        return EXIT_FAILURE;
    }

    const auto platform = config::get_platform();

    std::cout << std::format("Platform: {}\n", to_string(platform));

    auto config = Config{};

    auto config_builder = ConfigBuilder{};

    auto result = config_builder.with_defaults(platform, std::getenv).build(config);

    if (result != ConfigBuilder::Result::Success) {
        std::cerr << std::format("Error: {}\n", to_string(result));
        return EXIT_FAILURE;
    }

    std::cout << std::format("Config dir: {}\n", config.config_dir.string());
    std::cout << std::format("Data dir: {}\n", config.data_dir.string());

    return EXIT_SUCCESS;
}
