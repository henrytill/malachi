#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "test_utils.h"

#include "platform.h"

TEST_CASE("Platform detection") {
  auto platform = platform::get_platform();

#if defined(_WIN32)
  CHECK(platform == platform::Platform::Windows);
#elif defined(__APPLE__)
  CHECK(platform == platform::Platform::MacOS);
#elif defined(__linux__)
  CHECK(platform == platform::Platform::Linux);
#else
  CHECK(platform == platform::Platform::Unknown);
#endif
}

TEST_CASE("Platform string conversion") {
  CHECK(platform::to_string_view(platform::Platform::Windows) == "Windows");
  CHECK(platform::to_string_view(platform::Platform::MacOS) == "macOS");
  CHECK(platform::to_string_view(platform::Platform::Linux) == "Linux");
  CHECK(platform::to_string_view(platform::Platform::Unknown) == "Unknown");
}

TEST_CASE("Directory resolution") {
  auto env = test::MockEnvironment{};
  const auto getenv = [&env](const char *name) { return env.get(name); };
  const auto name = std::filesystem::path{"test_app"};

  SUBCASE("Windows paths") {
    env.set("LOCALAPPDATA", R"(C:\Users\Test\AppData\Local)");
    env.set("APPDATA", R"(C:\Users\Test\AppData\Roaming)");

    auto config_dir = platform::windows::get_app_data(getenv, name);
    REQUIRE(config_dir.has_value());
    const auto expected = std::filesystem::path{R"(C:\Users\Test\AppData\Roaming)"} / name;
    CHECK(*config_dir == expected);

    auto data_dir = platform::windows::get_local_app_data(getenv, name);
    REQUIRE(data_dir.has_value());
    const auto expected_data = std::filesystem::path{R"(C:\Users\Test\AppData\Local)"} / name;
    CHECK(*data_dir == expected_data);
  }

  SUBCASE("MacOS paths") {
    env.clear();
    env.set("HOME", "/Users/test");

    auto config_dir = platform::mac_os::get_application_support(getenv, name);
    REQUIRE(config_dir.has_value());
    const auto expected = std::filesystem::path{"/Users/test/Library/Application Support"} / name;
    CHECK(*config_dir == expected);
  }

  SUBCASE("XDG paths") {
    env.clear();
    env.set("XDG_CONFIG_HOME", "/home/test/.config");
    env.set("XDG_DATA_HOME", "/home/test/.local/share");

    auto config_dir = platform::xdg::get_config_home(getenv, name);
    REQUIRE(config_dir.has_value());
    const auto expected = std::filesystem::path{"/home/test/.config"} / name;
    CHECK(*config_dir == expected);

    auto data_dir = platform::xdg::get_data_home(getenv, name);
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

    auto config_dir = platform::windows::get_app_data(getenv, name);
    REQUIRE(config_dir.has_value());
    const auto expected = std::filesystem::path{R"(C:\Users\Test\AppData\Roaming)"} / name;
    CHECK(*config_dir == expected);

    auto data_dir = platform::windows::get_local_app_data(getenv, name);
    REQUIRE(data_dir.has_value());
    const auto expected_data = std::filesystem::path{R"(C:\Users\Test\AppData\Local)"} / name;
    CHECK(*data_dir == expected_data);
  }

  SUBCASE("MacOS paths") {
    env.clear();
    env.set("HOME", "/Users/test");

    auto config_dir = platform::mac_os::get_application_support(getenv, name);
    REQUIRE(config_dir.has_value());

    const auto expected = std::filesystem::path{"/Users/test/Library/Application Support"} / name;
    CHECK(*config_dir == expected);
  }

  SUBCASE("XDG paths") {
    env.clear();
    env.set("XDG_CONFIG_HOME", "/home/test/.config");
    env.set("XDG_DATA_HOME", "/home/test/.local/share");

    auto config_dir = platform::xdg::get_config_home(getenv, name);
    REQUIRE(config_dir.has_value());
    const auto expected = std::filesystem::path{"/home/test/.config"} / name;
    CHECK(*config_dir == expected);

    auto data_dir = platform::xdg::get_data_home(getenv, name);
    REQUIRE(data_dir.has_value());
    const auto expected_data = std::filesystem::path{"/home/test/.local/share"} / name;
    CHECK(*data_dir == expected_data);
  }
}
