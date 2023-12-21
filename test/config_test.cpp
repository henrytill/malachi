#include <filesystem>
#include <string>

#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>

#include "Config.h"
#include "Platform.h"

namespace config = malachi::config;

using std::filesystem::path;

using config::Config;
using config::ConfigBuilder;
using config::Platform;

// NOLINTBEGIN(misc-use-anonymous-namespace, cppcoreguidelines-avoid-do-while)

TEST_CASE("to_string returns the correct string for each platform", "[Platform]") {
  REQUIRE(config::to_string(Platform::Windows) == "Windows");
  REQUIRE(config::to_string(Platform::MacOS) == "MacOS");
  REQUIRE(config::to_string(Platform::Linux) == "Linux");
  REQUIRE(config::to_string(Platform::Unknown) == "Unknown");
}

TEST_CASE("ConfigBuilder builds a Config with the correct config and data directories on Linux", "[ConfigBuilder]") {
  const auto getenv = [](const char *name) noexcept -> char * {
    if (strcmp(name, "HOME") == 0) {
      constexpr auto ret = "/home/user";
      return const_cast<char *>(ret); // NOLINT(cppcoreguidelines-pro-type-const-cast)
    }
    return nullptr;
  };

  auto config = Config{};
  auto config_builder = ConfigBuilder{};
  auto result = config_builder.with_defaults(Platform::Linux, getenv).build(config);
  REQUIRE(result == ConfigBuilder::Result::Success);
  REQUIRE(config.config_dir == path{"/home/user/.config/malachi"});
  REQUIRE(config.data_dir == path{"/home/user/.local/share/malachi"});
}

// NOLINTEND(misc-use-anonymous-namespace, cppcoreguidelines-avoid-do-while)

auto main(int argc, char *argv[]) -> int {
  const int result = Catch::Session().run(argc, argv);
  return result;
}
