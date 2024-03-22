#include <filesystem>
#include <string>

#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>

#include "config.h"
#include "platform.h"

namespace config = malachi::config;

using std::filesystem::path;

using config::Config;
using config::ConfigBuilder;
using platform::Platform;

path get_root_directory() { return std::filesystem::current_path().root_directory(); }

// NOLINTBEGIN(misc-use-anonymous-namespace, cppcoreguidelines-avoid-do-while, readability-function-cognitive-complexity)

TEST_CASE("to_string returns the correct string for each platform", "[Platform]")
{
    REQUIRE(platform::to_string(Platform::Windows) == "Windows");
    REQUIRE(platform::to_string(Platform::MacOS) == "MacOS");
    REQUIRE(platform::to_string(Platform::Linux) == "Linux");
    REQUIRE(platform::to_string(Platform::Unknown) == "Unknown");
}

template <Platform p>
char *getenv_mock(const char *name) noexcept
{
    if constexpr (p == Platform::Windows) {
        if (strcmp(name, "APPDATA") == 0) {
            constexpr auto ret = R"(C:\Users\test\AppData\Roaming)";
            return const_cast<char *>(ret); // NOLINT(cppcoreguidelines-pro-type-const-cast)
        }
        if (strcmp(name, "LOCALAPPDATA") == 0) {
            constexpr auto ret = R"(C:\Users\test\AppData\Local)";
            return const_cast<char *>(ret); // NOLINT(cppcoreguidelines-pro-type-const-cast)
        }
        return nullptr;
    } else if constexpr (p == Platform::MacOS) {
        if (strcmp(name, "HOME") == 0) {
            constexpr auto ret = "/Users/test";
            return const_cast<char *>(ret); // NOLINT(cppcoreguidelines-pro-type-const-cast)
        }
        return nullptr;
    } else {
        if (strcmp(name, "HOME") == 0) {
            constexpr auto ret = "/home/test";
            return const_cast<char *>(ret); // NOLINT(cppcoreguidelines-pro-type-const-cast)
        }
        return nullptr;
    }
}

TEST_CASE("ConfigBuilder builds a Config with the correct config and data directories", "[ConfigBuilder]")
{
    constexpr Platform platform = platform::get_platform();

    auto config = Config{};
    auto config_builder = ConfigBuilder{};
    auto result = config_builder.with_defaults(getenv_mock<platform>).build(config);
    REQUIRE(result == ConfigBuilder::Result::Success);

    if constexpr (platform == Platform::Windows) {
        const auto root_dir = get_root_directory();
        const auto home_dir = path{root_dir / "Users"};
        REQUIRE(config.config_dir == path{home_dir / "test" / "AppData" / "Roaming" / "malachi"});
        REQUIRE(config.data_dir == path{home_dir / "test" / "AppData" / "Local" / "malachi"});
    } else if constexpr (platform == Platform::MacOS) {
        const auto root_dir = get_root_directory();
        const auto home_dir = path{root_dir / "Users"};
        REQUIRE(config.config_dir == path{home_dir / "test" / "Library" / "Application Support" / "malachi"});
        REQUIRE(config.data_dir == path{home_dir / "test" / "Library" / "Application Support" / "malachi"});
    } else {
        const auto root_dir = get_root_directory();
        const auto home_dir = path{root_dir / "home"};
        REQUIRE(config.config_dir == path{home_dir / "test" / ".config" / "malachi"});
        REQUIRE(config.data_dir == path{home_dir / "test" / ".local" / "share" / "malachi"});
    }
}

// NOLINTEND(misc-use-anonymous-namespace, cppcoreguidelines-avoid-do-while, readability-function-cognitive-complexity)

int main(int argc, char *argv[])
{
    const int result = Catch::Session().run(argc, argv);
    return result;
}
