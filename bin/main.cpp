#include <cstdlib>
#include <format>
#include <iostream>
#include <string>
#include <vector>

#include <poppler/cpp/poppler-version.h>
#include <sqlite3.h>

#include "config.h"
#include "platform.h"

using malachi::config::Config;
using malachi::config::ConfigBuilder;

int main(int argc, char *argv[])
{
    const std::vector<std::string_view> args(argv, argv + argc);
    if (args.size() != 2) {
        std::cerr << std::format("Usage: {} <filename>\n", args[0]);
        return EXIT_FAILURE;
    }

    auto config = Config{};
    auto config_builder = ConfigBuilder("malachi");
    auto result = config_builder.with_defaults(std::getenv).build(config);
    if (result != ConfigBuilder::Result::Success) {
        std::cerr << std::format("Error: {}\n", to_string_view(result));
        return EXIT_FAILURE;
    }

    constexpr auto platform = platform::get_platform();
    const std::string poppler_version = poppler::version_string();
    const std::string_view sqlite3_version = std::string_view{sqlite3_libversion()};
    std::cout << std::format("Platform: {}\n", to_string_view(platform));
    std::cout << std::format("Config dir: {}\n", config.config_dir.string());
    std::cout << std::format("Data dir: {}\n", config.data_dir.string());
    std::cout << std::format("Poppler version: {}\n", poppler_version);
    std::cout << std::format("SQLite3 version: {}\n", sqlite3_version);

    return EXIT_SUCCESS;
}
