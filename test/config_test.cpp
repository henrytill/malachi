#include <catch2/catch_test_macros.hpp>

#include "Platform.h"

namespace config = malachi::config;

TEST_CASE("to_string returns the correct string for each platform", "[Platform]") {
  using malachi::config::Platform;

  REQUIRE(config::to_string(Platform::Windows) == "Windows");
  REQUIRE(config::to_string(Platform::MacOS) == "MacOS");
  REQUIRE(config::to_string(Platform::Linux) == "Linux");
  REQUIRE(config::to_string(Platform::Unknown) == "Unknown");
}
