#pragma once

#include <cassert>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#ifndef _WIN32
#    include <unistd.h>
#endif

namespace platform
{

template <typename T>
using optional = std::optional<T>;

using path = std::filesystem::path;

using GetEnvFn = std::function<char *(char const *)>;

enum class Platform : uint8_t
{
    Windows,
    MacOS,
    Linux,
    Unknown,
};

#if defined(_WIN32)
[[nodiscard]] constexpr auto get_platform() -> Platform { return Platform::Windows; }
#elif defined(__APPLE__)
[[nodiscard]] constexpr auto get_platform() -> Platform { return Platform::MacOS; }
#elif defined(__linux__)
[[nodiscard]] constexpr auto get_platform() -> Platform { return Platform::Linux; }
#else
[[nodiscard]] constexpr auto get_platform() -> Platform { return Platform::Unknown; }
#endif

[[nodiscard]]
constexpr auto to_string_view(const Platform platform) -> std::string_view
{
    switch (platform)
    {
    case Platform::Windows:
        return "Windows";
    case Platform::MacOS:
        return "macOS";
    case Platform::Linux:
        return "Linux";
    case Platform::Unknown:
        [[fallthrough]];
    default:
        return "Unknown";
    };
}

namespace windows
{

[[nodiscard]]
inline auto get_app_data(GetEnvFn getenv, std::string_view const name) -> optional<path>
{
    auto app_data = std::unique_ptr<char> { getenv("APPDATA") };
    if (app_data != nullptr)
    {
        auto const config_dir = path { app_data.release() };
        return optional<path> { config_dir / name };
    }
    return std::nullopt;
}

[[nodiscard]]
inline auto get_local_app_data(GetEnvFn getenv, std::string_view const name) -> optional<path>
{
    auto local_app_data = std::unique_ptr<char> { getenv("LOCALAPPDATA") };
    if (local_app_data != nullptr)
    {
        auto const data_dir = path { local_app_data.release() };
        return optional<path> { data_dir / name };
    }
    return std::nullopt;
}

[[nodiscard]]
inline auto get_local_app_data_cache(GetEnvFn getenv, std::string_view const name) -> optional<path>
{
    auto local_app_data = std::unique_ptr<char> { getenv("LOCALAPPDATA") };
    if (local_app_data != nullptr)
    {
        auto const cache_dir = path { local_app_data.release() };
        return optional<path> { cache_dir / name };
    }
    return std::nullopt;
}

[[nodiscard]]
inline auto get_runtime_dir(GetEnvFn getenv, std::string_view const name) -> optional<path>
{
    auto temp = std::unique_ptr<char> { getenv("TEMP") };
    if (temp != nullptr)
    {
        auto const runtime_dir = path { temp.release() };
        return optional<path> { runtime_dir / name };
    }
    return std::nullopt;
}

} // namespace windows

namespace mac_os
{

[[nodiscard]]
inline auto get_application_support(GetEnvFn getenv, std::string_view const name) -> optional<path>
{
    auto home = std::unique_ptr<char> { getenv("HOME") };
    if (home != nullptr)
    {
        auto const home_dir = path { home.release() };
        return optional<path> { home_dir / "Library" / "Application Support" / name };
    }
    return std::nullopt;
}

[[nodiscard]]
inline auto get_caches(GetEnvFn getenv, std::string_view const name) -> optional<path>
{
    auto home = std::unique_ptr<char> { getenv("HOME") };
    if (home != nullptr)
    {
        auto const home_dir = path { home.release() };
        return optional<path> { home_dir / "Library" / "Caches" / name };
    }
    return std::nullopt;
}

[[nodiscard]]
inline auto get_runtime_dir(GetEnvFn getenv, std::string_view const name) -> optional<path>
{
    auto tmpdir = std::unique_ptr<char> { getenv("TMPDIR") };
    if (tmpdir != nullptr)
    {
        auto const tmp_dir = path { tmpdir.release() };
        return optional<path> { tmp_dir / name };
    }
    return optional<path> { path { "/tmp" } / name };
}

} // namespace mac_os

namespace xdg
{

[[nodiscard]]
inline auto get_config_home(GetEnvFn getenv, std::string_view const name) -> optional<path>
{
    auto xdg_config_home = std::unique_ptr<char> { getenv("XDG_CONFIG_HOME") };
    if (xdg_config_home != nullptr)
    {
        auto const config_dir = path { xdg_config_home.release() };
        return optional<path> { config_dir / name };
    }
    auto home = std::unique_ptr<char> { getenv("HOME") };
    if (home != nullptr)
    {
        auto const home_dir = path { home.release() };
        return optional<path> { home_dir / ".config" / name };
    }
    return std::nullopt;
}

[[nodiscard]]
inline auto get_data_home(GetEnvFn getenv, std::string_view const name) -> optional<path>
{
    auto xdg_data_home = std::unique_ptr<char> { getenv("XDG_DATA_HOME") };
    if (xdg_data_home != nullptr)
    {
        auto const data_dir = path { xdg_data_home.release() };
        return optional<path> { data_dir / name };
    }
    auto home = std::unique_ptr<char> { getenv("HOME") };
    if (home != nullptr)
    {
        auto const home_dir = path { home.release() };
        return optional<path> { home_dir / ".local" / "share" / name };
    }
    return std::nullopt;
}

[[nodiscard]]
inline auto get_cache_home(GetEnvFn getenv, std::string_view const name) -> optional<path>
{
    auto xdg_cache_home = std::unique_ptr<char> { getenv("XDG_CACHE_HOME") };
    if (xdg_cache_home != nullptr)
    {
        auto const cache_dir = path { xdg_cache_home.release() };
        return optional<path> { cache_dir / name };
    }
    auto home = std::unique_ptr<char> { getenv("HOME") };
    if (home != nullptr)
    {
        auto const home_dir = path { home.release() };
        return optional<path> { home_dir / ".cache" / name };
    }
    return std::nullopt;
}

[[nodiscard]]
inline auto get_runtime_dir(GetEnvFn getenv, std::string_view const name) -> optional<path>
{
    auto xdg_runtime_dir = std::unique_ptr<char> { getenv("XDG_RUNTIME_DIR") };
    if (xdg_runtime_dir != nullptr)
    {
        auto const runtime_dir = path { xdg_runtime_dir.release() };
        return optional<path> { runtime_dir / name };
    }
#ifndef _WIN32
    auto const uid = ::getuid();
    return optional<path> { path { "/run/user" } / std::to_string(uid) / name };
#else
    return std::nullopt;
#endif
}

} // namespace xdg

template <Platform p = get_platform()>
[[nodiscard]]
auto get_config_dir(GetEnvFn getenv, std::string_view const name) -> optional<path>
{
    if constexpr (p == Platform::Windows)
    {
        return windows::get_app_data(getenv, name);
    }
    else if constexpr (p == Platform::MacOS)
    {
        return mac_os::get_application_support(getenv, name);
    }
    else
    {
        return xdg::get_config_home(getenv, name);
    }
}

template <Platform p = get_platform()>
[[nodiscard]]
auto get_data_dir(GetEnvFn getenv, std::string_view const name) -> optional<path>
{
    if constexpr (p == Platform::Windows)
    {
        return windows::get_local_app_data(getenv, name);
    }
    else if constexpr (p == Platform::MacOS)
    {
        return mac_os::get_application_support(getenv, name);
    }
    else
    {
        return xdg::get_data_home(getenv, name);
    }
}

template <Platform p = get_platform()>
[[nodiscard]]
auto get_cache_dir(GetEnvFn getenv, std::string_view const name) -> optional<path>
{
    if constexpr (p == Platform::Windows)
    {
        return windows::get_local_app_data_cache(getenv, name);
    }
    else if constexpr (p == Platform::MacOS)
    {
        return mac_os::get_caches(getenv, name);
    }
    else
    {
        return xdg::get_cache_home(getenv, name);
    }
}

template <Platform p = get_platform()>
[[nodiscard]]
auto get_runtime_dir(GetEnvFn getenv, std::string_view const name) -> optional<path>
{
    if constexpr (p == Platform::Windows)
    {
        return windows::get_runtime_dir(getenv, name);
    }
    else if constexpr (p == Platform::MacOS)
    {
        return mac_os::get_runtime_dir(getenv, name);
    }
    else
    {
        return xdg::get_runtime_dir(getenv, name);
    }
}

} // namespace platform
