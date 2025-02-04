#include <catch2/catch_session.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

#include "environment.h"

#include "config.h"
#include "platform.h"

using namespace malachi::config;
using platform::Platform;

struct EmptyConfigFixture : Environment<EmptyConfigFixture> {
  static constexpr auto env = std::array<std::pair<const char *, const char *>, 0>{};
};

TEST_CASE_METHOD(EmptyConfigFixture, "Builder fails without configuration directory", "[config]") {
  auto result = Builder(EmptyConfigFixture::getenv).with_defaults().build();
  REQUIRE(std::holds_alternative<Error>(result));

  const auto &error = std::get<Error>(result);
  CHECK(error.code == ErrorCode::kMissingDir);
  CHECK(error.message == "Configuration directory could not be determined");
}

template <Platform P>
struct ConfigFixture;

template <>
struct ConfigFixture<Platform::Windows> : Environment<ConfigFixture<Platform::Windows>> {
  static constexpr auto env = std::array{
      std::pair{"APPDATA", R"(C:\Users\Test\AppData\Roaming)"},
      std::pair{"LOCALAPPDATA", R"(C:\Users\Test\AppData\Local)"}};
  static constexpr auto expected_config_dir = R"(C:\Users\Test\AppData\Roaming\malachi)";
  static constexpr auto expected_data_dir = R"(C:\Users\Test\AppData\Local\malachi)";
};

template <>
struct ConfigFixture<Platform::MacOS> : Environment<ConfigFixture<Platform::MacOS>> {
  static constexpr auto env = std::array{
      std::pair{"HOME", "/Users/test"}};
  static constexpr auto expected_config_dir = "/Users/test/Library/Application Support/malachi";
  static constexpr auto expected_data_dir = "/Users/test/Library/Application Support/malachi";
};

template <>
struct ConfigFixture<Platform::Linux> : Environment<ConfigFixture<Platform::Linux>> {
  static constexpr auto env = std::array{
      std::pair{"XDG_CONFIG_HOME", "/home/test/.config"},
      std::pair{"XDG_DATA_HOME", "/home/test/.local/share"}};
  static constexpr auto expected_config_dir = "/home/test/.config/malachi";
  static constexpr auto expected_data_dir = "/home/test/.local/share/malachi";
};

TEMPLATE_TEST_CASE_METHOD_SIG(ConfigFixture,
                              "Builder succeeds with platform-specific environment variables",
                              "[config]",
                              ((Platform P), P),
                              platform::get_platform()) {
  auto result = Builder(ConfigFixture<P>::getenv).with_defaults().build();
  REQUIRE(std::holds_alternative<Config>(result));

  const auto &config = std::get<Config>(result);
  CHECK(config.config_dir == std::filesystem::path{ConfigFixture<P>::expected_config_dir});
  CHECK(config.data_dir == std::filesystem::path{ConfigFixture<P>::expected_data_dir});
}

template <Platform P>
struct PartialConfigFixture;

template <>
struct PartialConfigFixture<Platform::Windows> : Environment<PartialConfigFixture<Platform::Windows>> {
  static constexpr auto env = std::array{
      std::pair{"APPDATA", R"(C:\Users\Test\AppData\Roaming)"}};
};

template <>
struct PartialConfigFixture<Platform::MacOS> : Environment<PartialConfigFixture<Platform::MacOS>> {
  static constexpr auto env = std::array{
      std::pair{"HOME", "/Users/test"}};
};

template <>
struct PartialConfigFixture<Platform::Linux> : Environment<PartialConfigFixture<Platform::Linux>> {
  static constexpr auto env = std::array{
      std::pair{"XDG_CONFIG_HOME", "/home/test/.config"}};
};

TEMPLATE_TEST_CASE_METHOD_SIG(PartialConfigFixture,
                              "Builder fails with partial configuration",
                              "[config]",
                              ((Platform P), P),
                              platform::get_platform()) {
  auto result = Builder(PartialConfigFixture<P>::getenv).with_defaults().build();

  if constexpr (P == Platform::MacOS) {
    // macOS doesn't require a separate data directory
    REQUIRE(std::holds_alternative<Config>(result));
  } else {
    // Other platforms require both directories
    REQUIRE(std::holds_alternative<Error>(result));

    const auto &error = std::get<Error>(result);
    CHECK(error.code == ErrorCode::kMissingDir);
    CHECK(error.message == "Data directory could not be determined");
  }
}

auto main(int argc, char *argv[]) -> int {
  const int result = Catch::Session().run(argc, argv);
  return result;
}
