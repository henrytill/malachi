#include <filesystem>
#include <format>
#include <string>

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

struct DirTestParams {
  Platform platform;
  std::vector<std::pair<std::string, std::string>> env;
  std::filesystem::path expected_config_base;
  std::filesystem::path expected_data_base;
};

namespace {

// NOLINTBEGIN(bugprone-easily-swappable-parameters)
template <Platform P>
inline auto verify_directories(const GetEnvFn &getenv,
                               const std::filesystem::path &name,
                               const std::filesystem::path &expected_config,
                               const std::filesystem::path &expected_data)
    -> bool {
  const auto config_dir = get_config_dir<P>(getenv, name);
  REQUIRE(config_dir.has_value());
  CHECK(config_dir.value() == expected_config);

  const auto data_dir = get_data_dir<P>(getenv, name);
  REQUIRE(data_dir.has_value());
  CHECK(data_dir.value() == expected_data);

  return true;
}
// NOLINTEND(bugprone-easily-swappable-parameters)

template <Platform... Ps>
void dispatch_verify_directories(const Platform platform,
                                 const GetEnvFn &getenv,
                                 const std::filesystem::path &name,
                                 const std::filesystem::path &expected_config,
                                 const std::filesystem::path &expected_data) {
  const auto found = ((platform == Ps && verify_directories<Ps>(getenv, name, expected_config, expected_data)) || ...);
  REQUIRE(found);
}

} // namespace

TEST_CASE("Directory resolution") {
  const auto params = std::vector<DirTestParams>{
      {.platform = Platform::Windows,
       .env = {{"LOCALAPPDATA", R"(C:\Users\Test\AppData\Local)"},
               {"APPDATA", R"(C:\Users\Test\AppData\Roaming)"}},
       .expected_config_base = R"(C:\Users\Test\AppData\Roaming)",
       .expected_data_base = R"(C:\Users\Test\AppData\Local)"},
      {.platform = Platform::MacOS,
       .env = {{"HOME", "/Users/test"}},
       .expected_config_base = "/Users/test/Library/Application Support",
       .expected_data_base = "/Users/test/Library/Application Support"},
      {.platform = Platform::Linux,
       .env = {{"XDG_CONFIG_HOME", "/home/test/.config"},
               {"XDG_DATA_HOME", "/home/test/.local/share"}},
       .expected_config_base = "/home/test/.config",
       .expected_data_base = "/home/test/.local/share"}};

  for (const auto &param : params) {
    const auto platform_str = std::format("Platform: {}", to_string_view(param.platform));

    SUBCASE(platform_str.c_str()) {
      auto env = test::MockEnvironment{};
      const auto getenv = [&env](const char *name) { return env.get(name); };

      for (const auto &name : {std::filesystem::path{"test_app"}, std::filesystem::path{}}) {
        const auto name_str = std::format("Name: {}", name.empty() ? "empty" : name.string());

        SUBCASE(name_str.c_str()) {
          env.clear();
          for (const auto &[key, value] : param.env) {
            env.set(key, value);
          }

          const auto expected_config = param.expected_config_base / name;
          const auto expected_data = param.expected_data_base / name;

          dispatch_verify_directories<Platform::Windows, Platform::MacOS, Platform::Linux>(
              param.platform, getenv, name, expected_config, expected_data);
        }
      }
    }
  }
}
