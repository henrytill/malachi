#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "test_utils.h"

#include "platform.h"

using namespace platform;

#if defined(_WIN32)
TEST_CASE("Platform detection: Windows") {
  CHECK(get_platform() == Platform::Windows);
}
#elif defined(__APPLE__)
TEST_CASE("Platform detection: MacOS") {
  CHECK(get_platform() == Platform::MacOS);
}
#elif defined(__linux__)
TEST_CASE("Platform detection: Linux") {
  CHECK(get_platform() == Platform::Linux);
}
#else
TEST_CASE("Platform detection: Unknown") {
  CHECK(get_platform() == Platform::Unknown);
}
#endif

TEST_CASE("Platform string conversion") {
  CHECK(to_string_view(Platform::Windows) == "Windows");
  CHECK(to_string_view(Platform::MacOS) == "macOS");
  CHECK(to_string_view(Platform::Linux) == "Linux");
  CHECK(to_string_view(Platform::Unknown) == "Unknown");
}

TEST_CASE("Directory resolution") {
  auto env = test::MockEnvironment{};
  const auto getenv = [&env](const char *name) { return env.get(name); };
  const auto name = std::filesystem::path{"test_app"};

  SUBCASE("Windows paths") {
    env.set("LOCALAPPDATA", R"(C:\Users\Test\AppData\Local)");
    env.set("APPDATA", R"(C:\Users\Test\AppData\Roaming)");

    auto config_dir = get_config_dir<Platform::Windows>(getenv, name);
    REQUIRE(config_dir.has_value());
    const auto expected = std::filesystem::path{R"(C:\Users\Test\AppData\Roaming)"} / name;
    CHECK(*config_dir == expected);

    auto data_dir = get_data_dir<Platform::Windows>(getenv, name);
    REQUIRE(data_dir.has_value());
    const auto expected_data = std::filesystem::path{R"(C:\Users\Test\AppData\Local)"} / name;
    CHECK(*data_dir == expected_data);
  }

  SUBCASE("MacOS paths") {
    env.clear();
    env.set("HOME", "/Users/test");

    auto config_dir = get_config_dir<Platform::MacOS>(getenv, name);
    REQUIRE(config_dir.has_value());
    const auto expected = std::filesystem::path{"/Users/test/Library/Application Support"} / name;
    CHECK(*config_dir == expected);

    auto data_dir = get_config_dir<Platform::MacOS>(getenv, name);
    REQUIRE(data_dir.has_value());
    const auto expected_data = std::filesystem::path{"/Users/test/Library/Application Support"} / name;
    CHECK(*data_dir == expected_data);
  }

  SUBCASE("XDG paths") {
    env.clear();
    env.set("XDG_CONFIG_HOME", "/home/test/.config");
    env.set("XDG_DATA_HOME", "/home/test/.local/share");

    auto config_dir = get_config_dir<Platform::Linux>(getenv, name);
    REQUIRE(config_dir.has_value());
    const auto expected = std::filesystem::path{"/home/test/.config"} / name;
    CHECK(*config_dir == expected);

    auto data_dir = get_data_dir<Platform::Linux>(getenv, name);
    REQUIRE(data_dir.has_value());
    const auto expected_data = std::filesystem::path{"/home/test/.local/share"} / name;
    CHECK(*data_dir == expected_data);
  }
}

TEST_CASE("Directory resolution with empty name") {
  auto env = test::MockEnvironment{};
  const auto getenv = [&env](const char *name) { return env.get(name); };
  const auto name = std::filesystem::path{};

  CHECK(name.empty());

  SUBCASE("Windows paths") {
    env.set("LOCALAPPDATA", R"(C:\Users\Test\AppData\Local)");
    env.set("APPDATA", R"(C:\Users\Test\AppData\Roaming)");

    auto config_dir = get_config_dir<Platform::Windows>(getenv, name);
    REQUIRE(config_dir.has_value());
    const auto expected = std::filesystem::path{R"(C:\Users\Test\AppData\Roaming)"} / name;
    CHECK(*config_dir == expected);

    auto data_dir = get_data_dir<Platform::Windows>(getenv, name);
    REQUIRE(data_dir.has_value());
    const auto expected_data = std::filesystem::path{R"(C:\Users\Test\AppData\Local)"} / name;
    CHECK(*data_dir == expected_data);
  }

  SUBCASE("MacOS paths") {
    env.clear();
    env.set("HOME", "/Users/test");

    auto config_dir = get_config_dir<Platform::MacOS>(getenv, name);
    REQUIRE(config_dir.has_value());
    const auto expected = std::filesystem::path{"/Users/test/Library/Application Support"} / name;
    CHECK(*config_dir == expected);

    auto data_dir = get_config_dir<Platform::MacOS>(getenv, name);
    REQUIRE(data_dir.has_value());
    const auto expected_data = std::filesystem::path{"/Users/test/Library/Application Support"} / name;
    CHECK(*data_dir == expected_data);
  }

  SUBCASE("XDG paths") {
    env.clear();
    env.set("XDG_CONFIG_HOME", "/home/test/.config");
    env.set("XDG_DATA_HOME", "/home/test/.local/share");

    auto config_dir = get_config_dir<Platform::Linux>(getenv, name);
    REQUIRE(config_dir.has_value());
    const auto expected = std::filesystem::path{"/home/test/.config"} / name;
    CHECK(*config_dir == expected);

    auto data_dir = get_data_dir<Platform::Linux>(getenv, name);
    REQUIRE(data_dir.has_value());
    const auto expected_data = std::filesystem::path{"/home/test/.local/share"} / name;
    CHECK(*data_dir == expected_data);
  }
}
