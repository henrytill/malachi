#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "test_utils.h"

#include "config.h"
#include "platform.h"

using namespace malachi::config;
using platform::Platform;

constexpr auto kPlatform = platform::get_platform();

TEST_CASE("Builder fails without configuration directory") {
  auto env = test::MockEnvironment{};
  const auto getenv = [&env](const char *name) { return env.get(name); };

  auto result = Builder(getenv).with_defaults().build();
  REQUIRE(std::holds_alternative<Error>(result));

  const auto &error = std::get<Error>(result);
  CHECK(error.code == ErrorCode::kMissingDir);
  CHECK(error.message == "Configuration directory could not be determined");
}

TEST_CASE("Builder fails without data directory"
          // macOS does not have separate config and data directories
          * doctest::skip(kPlatform == Platform::MacOS)) {
  auto env = test::MockEnvironment{};
  const auto getenv = [&env](const char *name) { return env.get(name); };

  if constexpr (kPlatform == Platform::Windows) {
    env.set("APPDATA", R"(C:\Users\Test\AppData\Roaming)");
  } else {
    env.set("XDG_CONFIG_HOME", "/home/test/.config");
  }

  auto result = Builder(getenv).with_defaults().build();
  REQUIRE(std::holds_alternative<Error>(result));

  const auto &error = std::get<Error>(result);
  CHECK(error.code == ErrorCode::kMissingDir);
  CHECK(error.message == "Data directory could not be determined");
}

TEST_CASE("Builder succeeds with the expected environment variables") {
  auto env = test::MockEnvironment{};
  const auto getenv = [&env](const char *name) { return env.get(name); };

  if constexpr (kPlatform == Platform::Windows) {
    env.set("APPDATA", R"(C:\Users\Test\AppData\Roaming)");
    env.set("LOCALAPPDATA", R"(C:\Users\Test\AppData\Local)");
  } else if constexpr (kPlatform == Platform::MacOS) {
    env.set("HOME", "/Users/test");
  } else {
    env.set("XDG_CONFIG_HOME", "/home/test/.config");
    env.set("XDG_DATA_HOME", "/home/test/.local/share");
  }

  auto result = Builder(getenv).with_defaults().build();
  REQUIRE(std::holds_alternative<Config>(result));

  const auto &config = std::get<Config>(result);
  CHECK_FALSE(config.config_dir.empty());
  CHECK_FALSE(config.data_dir.empty());
}
