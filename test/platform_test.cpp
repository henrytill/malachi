#include <array>
#include <filesystem>
#include <format>
#include <string_view>
#include <utility>

#include <catch2/catch_session.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

#include "environment.h"

#include "platform.h"

using namespace std::literals;

using namespace platform;

#if defined(_WIN32)
TEST_CASE("Platform detection: Windows", "[platform]") {
  CHECK(get_platform() == Platform::Windows);
}
#elif defined(__APPLE__)
TEST_CASE("Platform detection: MacOS", "[platform]") {
  CHECK(get_platform() == Platform::MacOS);
}
#elif defined(__linux__)
TEST_CASE("Platform detection: Linux", "[platform]") {
  CHECK(get_platform() == Platform::Linux);
}
#else
TEST_CASE("Platform detection: Unknown", "[platform]") {
  CHECK(get_platform() == Platform::Unknown);
}
#endif

TEST_CASE("Platform string conversion", "[platform]") {
  CHECK(to_string_view(Platform::Windows) == "Windows");
  CHECK(to_string_view(Platform::MacOS) == "macOS");
  CHECK(to_string_view(Platform::Linux) == "Linux");
  CHECK(to_string_view(Platform::Unknown) == "Unknown");
}

template <Platform P>
struct DirFixture;

template <>
struct DirFixture<Platform::Windows> : Environment<DirFixture<Platform::Windows>> {
  static constexpr auto config_base = R"(C:\Users\Test\AppData\Roaming)";
  static constexpr auto data_base = R"(C:\Users\Test\AppData\Local)";
  static constexpr auto env = std::array{
      std::pair{"APPDATA"sv, config_base},
      std::pair{"LOCALAPPDATA"sv, data_base}};
};

template <>
struct DirFixture<Platform::MacOS> : Environment<DirFixture<Platform::MacOS>> {
  static constexpr auto config_base = "/Users/test/Library/Application Support";
  static constexpr auto data_base = "/Users/test/Library/Application Support";
  static constexpr auto env = std::array{
      std::pair{"HOME"sv, "/Users/test"}};
};

template <>
struct DirFixture<Platform::Linux> : Environment<DirFixture<Platform::Linux>> {
  static constexpr auto config_base = "/home/test/.config";
  static constexpr auto data_base = "/home/test/.local/share";
  static constexpr auto env = std::array{
      std::pair{"XDG_CONFIG_HOME"sv, config_base},
      std::pair{"XDG_DATA_HOME"sv, data_base}};
};

static_assert(std::string_view{DirFixture<Platform::Linux>::getenv("XDG_CONFIG_HOME"sv)} == DirFixture<Platform::Linux>::config_base);

TEMPLATE_TEST_CASE_METHOD_SIG(DirFixture,
                              "Directory resolution", "[platform]",
                              ((Platform P), P),
                              Platform::Windows, Platform::MacOS, Platform::Linux) {
  for (auto const &name : {std::string_view{"test_app"}, std::string_view{}}) {
    SECTION(std::format("Name: {}", name.empty() ? "<empty>" : name)) {
      auto const expected_config = std::filesystem::path{DirFixture<P>::config_base} / name;
      auto const expected_data = std::filesystem::path{DirFixture<P>::data_base} / name;

      auto const config_dir = get_config_dir<P>(DirFixture<P>::getenv, name);
      REQUIRE(config_dir.has_value());
      CHECK(config_dir.value() == expected_config); // NOLINT(bugprone-unchecked-optional-access)

      auto const data_dir = get_data_dir<P>(DirFixture<P>::getenv, name);
      REQUIRE(data_dir.has_value());
      CHECK(data_dir.value() == expected_data); // NOLINT(bugprone-unchecked-optional-access)
    }
  }
}

auto main(int argc, char *argv[]) -> int {
  auto const result = Catch::Session().run(argc, argv);
  return result;
}
