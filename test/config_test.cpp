#include <array>
#include <filesystem>
#include <string_view>
#include <utility>
#include <variant>

#include <catch2/catch_session.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

#include "environment.h"

#include "config.h"
#include "platform.h"

using namespace std::literals;

using namespace malachi::config;
using platform::Platform;

struct EmptyConfigFixture : Environment<EmptyConfigFixture> {
    static constexpr auto env = std::array<std::pair<std::string_view, char const *>, 0>{};
};

TEST_CASE_METHOD(EmptyConfigFixture, "Builder fails without configuration directory", "[config]") {
    auto const result = Builder(EmptyConfigFixture::getenv).with_defaults().build();
    REQUIRE(std::holds_alternative<Error>(result));

    auto const &error = std::get<Error>(result);
    CHECK(error.code == ErrorCode::kMissingDir);
    CHECK(error.message == "Configuration directory could not be determined");
}

template <Platform P>
struct ConfigFixture;

template <>
struct ConfigFixture<Platform::Windows> : Environment<ConfigFixture<Platform::Windows>> {
    static constexpr auto env = std::array{
        std::pair{"APPDATA"sv, R"(C:\Users\Test\AppData\Roaming)"},
        std::pair{"LOCALAPPDATA"sv, R"(C:\Users\Test\AppData\Local)"}};
    static constexpr auto expected_config_dir = R"(C:\Users\Test\AppData\Roaming\malachi)";
    static constexpr auto expected_data_dir = R"(C:\Users\Test\AppData\Local\malachi)";
};

template <>
struct ConfigFixture<Platform::MacOS> : Environment<ConfigFixture<Platform::MacOS>> {
    static constexpr auto env = std::array{
        std::pair{"HOME"sv, "/Users/test"}};
    static constexpr auto expected_config_dir = "/Users/test/Library/Application Support/malachi";
    static constexpr auto expected_data_dir = "/Users/test/Library/Application Support/malachi";
};

template <>
struct ConfigFixture<Platform::Linux> : Environment<ConfigFixture<Platform::Linux>> {
    static constexpr auto env = std::array{
        std::pair{"XDG_CONFIG_HOME"sv, "/home/test/.config"},
        std::pair{"XDG_DATA_HOME"sv, "/home/test/.local/share"}};
    static constexpr auto expected_config_dir = "/home/test/.config/malachi";
    static constexpr auto expected_data_dir = "/home/test/.local/share/malachi";
};

TEMPLATE_TEST_CASE_METHOD_SIG(ConfigFixture,
                              "Builder succeeds with platform-specific environment variables",
                              "[config]",
                              ((Platform P), P),
                              platform::get_platform()) {
    using fixture = ConfigFixture<P>;

    auto const result = Builder(fixture::getenv).with_defaults().build();
    REQUIRE(std::holds_alternative<Config>(result));

    auto const &config = std::get<Config>(result);
    CHECK(config.config_dir == std::filesystem::path{fixture::expected_config_dir});
    CHECK(config.data_dir == std::filesystem::path{fixture::expected_data_dir});
}

template <Platform P>
struct PartialConfigFixture;

template <>
struct PartialConfigFixture<Platform::Windows> : Environment<PartialConfigFixture<Platform::Windows>> {
    static constexpr auto env = std::array{
        std::pair{"APPDATA"sv, R"(C:\Users\Test\AppData\Roaming)"}};
};

template <>
struct PartialConfigFixture<Platform::MacOS> : Environment<PartialConfigFixture<Platform::MacOS>> {
    static constexpr auto env = std::array{
        std::pair{"HOME"sv, "/Users/test"}};
};

template <>
struct PartialConfigFixture<Platform::Linux> : Environment<PartialConfigFixture<Platform::Linux>> {
    static constexpr auto env = std::array{
        std::pair{"XDG_CONFIG_HOME"sv, "/home/test/.config"}};
};

TEMPLATE_TEST_CASE_METHOD_SIG(PartialConfigFixture,
                              "Builder fails with partial configuration",
                              "[config]",
                              ((Platform P), P),
                              platform::get_platform()) {
    auto const result = Builder(PartialConfigFixture<P>::getenv).with_defaults().build();

    if constexpr (P == Platform::MacOS) {
        // macOS doesn't require a separate data directory
        REQUIRE(std::holds_alternative<Config>(result));
    } else {
        // Other platforms require both directories
        REQUIRE(std::holds_alternative<Error>(result));

        auto const &error = std::get<Error>(result);
        CHECK(error.code == ErrorCode::kMissingDir);
        CHECK(error.message == "Data directory could not be determined");
    }
}

auto main(int argc, char *argv[]) -> int {
    auto const result = Catch::Session().run(argc, argv);
    return result;
}
