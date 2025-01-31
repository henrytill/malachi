#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "test_utils.h"

#include "config.h"

using namespace malachi::config;

TEST_CASE("Builder fails without directories") {
  auto env = test::MockEnvironment{};

  auto result = Builder([&env](const char *name) { return env.get(name); }).build();
  REQUIRE(std::holds_alternative<Error>(result));

  const auto &error = std::get<Error>(result);
  CHECK(error.code == ErrorCode::kMissingDir);
  CHECK(error.message == "maybe_config_dir is empty");
}

TEST_CASE("Builder with environment variables") {
  auto env = test::MockEnvironment{};

#ifdef _WIN32
  env.set("APPDATA", "C:\\Users\\Test\\AppData\\Roaming");
  env.set("LOCALAPPDATA", "C:\\Users\\Test\\AppData\\Local");
#elif defined(__APPLE__)
  env.set("HOME", "/Users/test");
#else
  env.set("XDG_CONFIG_HOME", "/home/test/.config");
  env.set("XDG_DATA_HOME", "/home/test/.local/share");
#endif

  auto result = Builder([&env](const char *name) { return env.get(name); }).with_defaults().build();
  REQUIRE(std::holds_alternative<Config>(result));

  const auto &config = std::get<Config>(result);
  CHECK_FALSE(config.config_dir.empty());
  CHECK_FALSE(config.data_dir.empty());
}
