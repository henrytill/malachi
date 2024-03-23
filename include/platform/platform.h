#pragma once

#include <cassert>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <optional>
#include <string>

namespace platform {

using std::filesystem::path;

using GetEnvFn = std::function<char *(const char *)>;

enum class Platform : uint8_t {
    Windows,
    MacOS,
    Linux,
    Unknown,
};

constexpr Platform get_platform()
{
#if defined(_WIN32)
    return Platform::Windows;
#elif defined(__APPLE__)
    return Platform::MacOS;
#elif defined(__linux__)
    return Platform::Linux;
#else
    return Platform::Unknown;
#endif
}

constexpr std::string_view to_string_view(const Platform p)
{
    switch (p) {
    case Platform::Windows:
        return "Windows";
    case Platform::MacOS:
        return "MacOS";
    case Platform::Linux:
        return "Linux";
    case Platform::Unknown:
    default:
        return "Unknown";
    };
}

namespace windows {

inline std::optional<path> get_app_data(GetEnvFn getenv, const path name)
{
    auto app_data = std::unique_ptr<char>{getenv("APPDATA")};
    if (app_data != nullptr) {
        const auto config_dir = path{app_data.release()};
        return std::optional<path>{config_dir / name};
    }
    return std::nullopt;
}

inline std::optional<path> get_local_app_data(GetEnvFn getenv, const path name)
{
    auto local_app_data = std::unique_ptr<char>{getenv("LOCALAPPDATA")};
    if (local_app_data != nullptr) {
        const auto data_dir = path{local_app_data.release()};
        return std::optional<path>{data_dir / name};
    }
    return std::nullopt;
}

} // namespace windows

namespace mac_os {

inline std::optional<path> get_application_support(GetEnvFn getenv, const path name)
{
    auto home = std::unique_ptr<char>{getenv("HOME")};
    if (home != nullptr) {
        const auto home_dir = path{home.release()};
        return std::optional<path>{home_dir / "Library" / "Application Support" / name};
    }
    return std::nullopt;
}

} // namespace mac_os

namespace xdg {

inline std::optional<path> get_config_home(GetEnvFn getenv, const path name)
{
    auto xdg_config_home = std::unique_ptr<char>{getenv("XDG_CONFIG_HOME")};
    if (xdg_config_home != nullptr) {
        const auto config_dir = path{xdg_config_home.release()};
        return std::optional<path>{config_dir / name};
    }
    auto home = std::unique_ptr<char>{getenv("HOME")};
    if (home != nullptr) {
        const auto home_dir = path{home.release()};
        return std::optional<path>{home_dir / ".config" / name};
    }
    return std::nullopt;
}

inline std::optional<path> get_data_home(GetEnvFn getenv, const path name)
{
    auto xdg_data_home = std::unique_ptr<char>{getenv("XDG_DATA_HOME")};
    if (xdg_data_home != nullptr) {
        const auto data_dir = path{xdg_data_home.release()};
        return std::optional<path>{data_dir / name};
    }
    auto home = std::unique_ptr<char>{getenv("HOME")};
    if (home != nullptr) {
        const auto home_dir = path{home.release()};
        return std::optional<path>{home_dir / ".local" / "share" / name};
    }
    return std::nullopt;
}

} // namespace xdg

template <Platform p = get_platform()>
std::optional<path> get_config_dir(GetEnvFn getenv, const path name)
{
    assert(name.empty() == false);

    if constexpr (p == Platform::Windows) {
        return windows::get_app_data(getenv, name);
    } else if constexpr (p == Platform::MacOS) {
        return mac_os::get_application_support(getenv, name);
    } else {
        return xdg::get_config_home(getenv, name);
    }
}

template <Platform p = get_platform()>
std::optional<path> get_data_dir(GetEnvFn getenv, const path name)
{
    assert(name.empty() == false);

    if constexpr (p == Platform::Windows) {
        return windows::get_local_app_data(getenv, name);
    } else if constexpr (p == Platform::MacOS) {
        return mac_os::get_application_support(getenv, name);
    } else {
        return xdg::get_data_home(getenv, name);
    }
}

} // namespace platform
