#pragma once

#include <cassert>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>

using std::filesystem::path;

namespace platform {

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

constexpr std::string to_string(const Platform p)
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

using GetEnv = char *(const char *) noexcept;

inline std::optional<path> get_config_dir_windows(GetEnv getenv, const path name)
{
    auto app_data = std::unique_ptr<char>{getenv("APPDATA")};
    if (app_data != nullptr) {
        const auto config_dir = path{app_data.release()};
        return std::optional<path>{config_dir / name};
    }
    return std::nullopt;
}

inline std::optional<path> get_data_dir_windows(GetEnv getenv, const path name)
{
    auto local_app_data = std::unique_ptr<char>{getenv("LOCALAPPDATA")};
    if (local_app_data != nullptr) {
        const auto data_dir = path{local_app_data.release()};
        return std::optional<path>{data_dir / name};
    }
    return std::nullopt;
}

inline std::optional<path> get_support_dir_macos(GetEnv getenv, const path name)
{
    auto home = std::unique_ptr<char>{getenv("HOME")};
    if (home != nullptr) {
        const auto home_dir = path{home.release()};
        return std::optional<path>{home_dir / "Library" / "Application Support" / name};
    }
    return std::nullopt;
}

inline std::optional<path> get_config_dir_unixen(GetEnv getenv, const path name)
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

inline std::optional<path> get_data_dir_unixen(GetEnv getenv, const path name)
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

template <Platform p>
std::optional<path> get_config_dir(GetEnv getenv, const path name)
{
    assert(name.empty() == false);

    if constexpr (p == Platform::Windows) {
        return get_config_dir_windows(getenv, name);
    } else if constexpr (p == Platform::MacOS) {
        return get_support_dir_macos(getenv, name);
    } else {
        return get_config_dir_unixen(getenv, name);
    }
}

template <Platform p>
std::optional<path> get_data_dir(GetEnv getenv, const path name)
{
    assert(name.empty() == false);

    if constexpr (p == Platform::Windows) {
        return get_data_dir_windows(getenv, name);
    } else if constexpr (p == Platform::MacOS) {
        return get_support_dir_macos(getenv, name);
    } else {
        return get_data_dir_unixen(getenv, name);
    }
}

} // namespace platform
