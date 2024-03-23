#include <filesystem>

#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>

#include "platform.h"

using platform::Platform;

namespace {

std::filesystem::path get_root_directory() { return std::filesystem::current_path().root_directory(); }

TEST_CASE("to_string returns the correct string for each platform", "[platform]")
{
    REQUIRE(platform::to_string_view(Platform::Windows) == "Windows");
    REQUIRE(platform::to_string_view(Platform::MacOS) == "MacOS");
    REQUIRE(platform::to_string_view(Platform::Linux) == "Linux");
    REQUIRE(platform::to_string_view(Platform::Unknown) == "Unknown");
}

template <Platform p>
char *getenv_mock(bool use_xdg, const char *name)
{
    if constexpr (p == Platform::Windows) {
        if (strcmp(name, "APPDATA") == 0) {
            constexpr auto ret = R"(C:\Users\test\AppData\Roaming)";
            return const_cast<char *>(ret);
        }
        if (strcmp(name, "LOCALAPPDATA") == 0) {
            constexpr auto ret = R"(C:\Users\test\AppData\Local)";
            return const_cast<char *>(ret);
        }
        return nullptr;
    } else if constexpr (p == Platform::MacOS) {
        if (strcmp(name, "HOME") == 0) {
            constexpr auto ret = "/Users/test";
            return const_cast<char *>(ret);
        }
        return nullptr;
    } else {
        if (use_xdg && (strcmp(name, "XDG_CONFIG_HOME") == 0)) {
            constexpr auto ret = "/tmp/config";
            return const_cast<char *>(ret);
        }
        if (use_xdg && (strcmp(name, "XDG_DATA_HOME") == 0)) {
            constexpr auto ret = "/tmp/data";
            return const_cast<char *>(ret);
        }
        if (strcmp(name, "HOME") == 0) {
            constexpr auto ret = "/home/test";
            return const_cast<char *>(ret);
        }
        return nullptr;
    }
}

TEST_CASE("get_config_dir returns the correct path for each plaform", "[get_config_dir]")
{
    using std::filesystem::path;

    constexpr auto p = platform::get_platform();

    auto getenv = [](const char *name) { return getenv_mock<p>(false, name); };

    const auto name = path{"foo"};
    std::optional<path> config_dir = platform::get_config_dir(getenv, name);

    if constexpr (p == Platform::Windows) {
        const auto root_dir = get_root_directory();
        const auto home_dir = path{root_dir / "Users"};
        REQUIRE(config_dir == path{home_dir / "test" / "AppData" / "Roaming" / name});
    } else if constexpr (p == Platform::MacOS) {
        const auto root_dir = get_root_directory();
        const auto home_dir = path{root_dir / "Users"};
        REQUIRE(config_dir == path{home_dir / "test" / "Library" / "Application Support" / name});
    } else {
        const auto root_dir = get_root_directory();
        const auto home_dir = path{root_dir / "home"};
        REQUIRE(config_dir == path{home_dir / "test" / ".config" / name});
    }
}

TEST_CASE("get_data_dir returns the correct path for each plaform", "[get_data_dir]")
{
    using std::filesystem::path;

    constexpr auto p = platform::get_platform();

    auto getenv = [](const char *name) { return getenv_mock<p>(false, name); };

    const auto name = path{"foo"};
    std::optional<path> data_dir = platform::get_data_dir(getenv, name);

    if constexpr (p == Platform::Windows) {
        const auto root_dir = get_root_directory();
        const auto home_dir = path{root_dir / "Users"};
        REQUIRE(data_dir == path{home_dir / "test" / "AppData" / "Local" / name});
    } else if constexpr (p == Platform::MacOS) {
        const auto root_dir = get_root_directory();
        const auto home_dir = path{root_dir / "Users"};
        REQUIRE(data_dir == path{home_dir / "test" / "Library" / "Application Support" / name});
    } else {
        const auto root_dir = get_root_directory();
        const auto home_dir = path{root_dir / "home"};
        REQUIRE(data_dir == path{home_dir / "test" / ".local" / "share" / name});
    }
}

TEST_CASE("get_config_dir returns the correct path when XDG_CONFIG_HOME is set", "[get_config_dir]")
{
    using std::filesystem::path;
    constexpr auto p = platform::get_platform();
    if constexpr (p == Platform::Linux || p == Platform::Unknown) {
        auto getenv = [](const char *name) { return getenv_mock<p>(true, name); };
        const auto name = path{"foo"};
        std::optional<path> config_dir = platform::get_config_dir(getenv, name);
        const auto root_dir = get_root_directory();
        REQUIRE(config_dir == path{root_dir / "tmp" / "config" / name});
    } else {
        SKIP("only runnable on Linux and other Unixen");
    }
}

TEST_CASE("get_data_dir returns the correct path when XDG_DATA_HOME is set", "[get_data_dir]")
{
    using std::filesystem::path;
    constexpr auto p = platform::get_platform();
    if constexpr (p == Platform::Linux || p == Platform::Unknown) {
        auto getenv = [](const char *name) { return getenv_mock<p>(true, name); };
        const auto name = path{"foo"};
        std::optional<path> config_dir = platform::get_data_dir(getenv, name);
        const auto root_dir = get_root_directory();
        REQUIRE(config_dir == path{root_dir / "tmp" / "data" / name});
    } else {
        SKIP("only runnable on Linux and other Unixen");
    }
}

} // namespace

int main(int argc, char *argv[])
{
    const int result = Catch::Session().run(argc, argv);
    return result;
}
