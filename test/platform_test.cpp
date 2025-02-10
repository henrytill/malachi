#include <array>
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
  static constexpr auto env = std::array{
      std::pair{"LOCALAPPDATA"sv, R"(C:\Users\Test\AppData\Local)"},
      std::pair{"APPDATA"sv, R"(C:\Users\Test\AppData\Roaming)"}};
  static constexpr auto config_base = R"(C:\Users\Test\AppData\Roaming)";
  static constexpr auto data_base = R"(C:\Users\Test\AppData\Local)";
};

template <>
struct DirFixture<Platform::MacOS> : Environment<DirFixture<Platform::MacOS>> {
  static constexpr auto env = std::array{
      std::pair{"HOME"sv, "/Users/test"}};
  static constexpr auto config_base = "/Users/test/Library/Application Support";
  static constexpr auto data_base = "/Users/test/Library/Application Support";
};

template <>
struct DirFixture<Platform::Linux> : Environment<DirFixture<Platform::Linux>> {
  static constexpr auto env = std::array{
      std::pair{"XDG_CONFIG_HOME"sv, "/home/test/.config"},
      std::pair{"XDG_DATA_HOME"sv, "/home/test/.local/share"}};
  static constexpr auto config_base = "/home/test/.config";
  static constexpr auto data_base = "/home/test/.local/share";
};

static_assert(std::string_view{DirFixture<Platform::Linux>::getenv("XDG_CONFIG_HOME"sv)} == "/home/test/.config");

TEMPLATE_TEST_CASE_METHOD_SIG(DirFixture,
                              "Directory resolution", "[platform]",
                              ((Platform P), P),
                              Platform::Windows, Platform::MacOS, Platform::Linux) {
  for (const auto &name : {std::filesystem::path{"test_app"}, std::filesystem::path{}}) {
    SECTION(std::format("Name: {}", name.empty() ? "empty" : name.string())) {
      const auto expected_config = std::filesystem::path{DirFixture<P>::config_base} / name;
      const auto expected_data = std::filesystem::path{DirFixture<P>::data_base} / name;

      const auto config_dir = get_config_dir<P>(DirFixture<P>::getenv, name);
      REQUIRE(config_dir.has_value());
      CHECK(config_dir.value() == expected_config);

      const auto data_dir = get_data_dir<P>(DirFixture<P>::getenv, name);
      REQUIRE(data_dir.has_value());
      CHECK(data_dir.value() == expected_data);
    }
  }
}

auto main(int argc, char *argv[]) -> int {
  const int result = Catch::Session().run(argc, argv);
  return result;
}
