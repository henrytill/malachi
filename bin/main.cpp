#include <cstdlib>
#include <format>
#include <iostream>
#include <string>
#include <vector>

#include "config.h"
#include "platform.h"

using malachi::config::Config;
using malachi::config::ConfigBuilder;

int main(int argc, char *argv[])
{
    const std::vector<std::string> args(argv, argv + argc);
    if (args.size() != 2) {
        std::cerr << std::format("Usage: {} <filename>\n", args[0]);
        return EXIT_FAILURE;
    }

    auto config = Config{};
    auto config_builder = ConfigBuilder{};
    auto result = config_builder.with_defaults(std::getenv).build(config);
    if (result != ConfigBuilder::Result::Success) {
        std::cerr << std::format("Error: {}\n", to_string(result));
        return EXIT_FAILURE;
    }

    constexpr auto platform = platform::get_platform();
    std::cout << std::format("Platform: {}\n", to_string(platform));
    std::cout << std::format("Config dir: {}\n", config.config_dir.string());
    std::cout << std::format("Data dir: {}\n", config.data_dir.string());

    return EXIT_SUCCESS;
}
