#pragma once

#include <cassert>
#include <cstdint>
#include <filesystem>
#include <functional>
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
    if (auto const *app_data = getenv("APPDATA"); app_data != nullptr)
    {
        return optional<path> { path { app_data } / name };
    }
    return std::nullopt;
}

[[nodiscard]]
inline auto get_local_app_data(GetEnvFn getenv, std::string_view const name) -> optional<path>
{
    if (auto const *local_app_data = getenv("LOCALAPPDATA"); local_app_data != nullptr)
    {
        return optional<path> { path { local_app_data } / name };
    }
    return std::nullopt;
}

[[nodiscard]]
inline auto get_local_app_data_cache(GetEnvFn getenv, std::string_view const name) -> optional<path>
{
    if (auto const *local_app_data = getenv("LOCALAPPDATA"); local_app_data != nullptr)
    {
        return optional<path> { path { local_app_data } / name };
    }
    return std::nullopt;
}

[[nodiscard]]
inline auto get_runtime_dir(GetEnvFn getenv, std::string_view const name) -> optional<path>
{
    if (auto const *temp = getenv("TEMP"); temp != nullptr)
    {
        return optional<path> { path { temp } / name };
    }
    return std::nullopt;
}

} // namespace windows

namespace mac_os
{

[[nodiscard]]
inline auto get_application_support(GetEnvFn getenv, std::string_view const name) -> optional<path>
{
    if (auto const *home = getenv("HOME"); home != nullptr)
    {
        return optional<path> { path { home } / "Library" / "Application Support" / name };
    }
    return std::nullopt;
}

[[nodiscard]]
inline auto get_caches(GetEnvFn getenv, std::string_view const name) -> optional<path>
{
    if (auto const *home = getenv("HOME"); home != nullptr)
    {
        return optional<path> { path { home } / "Library" / "Caches" / name };
    }
    return std::nullopt;
}

[[nodiscard]]
inline auto get_runtime_dir(GetEnvFn getenv, std::string_view const name) -> optional<path>
{
    if (auto const *tmpdir = getenv("TMPDIR"); tmpdir != nullptr)
    {
        return optional<path> { path { tmpdir } / name };
    }
    return optional<path> { path { "/tmp" } / name };
}

} // namespace mac_os

namespace xdg
{

[[nodiscard]]
inline auto get_config_home(GetEnvFn getenv, std::string_view const name) -> optional<path>
{
    if (auto const *xdg_config_home = getenv("XDG_CONFIG_HOME"); xdg_config_home != nullptr)
    {
        return optional<path> { path { xdg_config_home } / name };
    }
    if (auto const *home = getenv("HOME"); home != nullptr)
    {
        return optional<path> { path { home } / ".config" / name };
    }
    return std::nullopt;
}

[[nodiscard]]
inline auto get_data_home(GetEnvFn getenv, std::string_view const name) -> optional<path>
{
    if (auto const *xdg_data_home = getenv("XDG_DATA_HOME"); xdg_data_home != nullptr)
    {
        return optional<path> { path { xdg_data_home } / name };
    }
    if (auto const *home = getenv("HOME"); home != nullptr)
    {
        return optional<path> { path { home } / ".local" / "share" / name };
    }
    return std::nullopt;
}

[[nodiscard]]
inline auto get_cache_home(GetEnvFn getenv, std::string_view const name) -> optional<path>
{
    if (auto const *xdg_cache_home = getenv("XDG_CACHE_HOME"); xdg_cache_home != nullptr)
    {
        return optional<path> { path { xdg_cache_home } / name };
    }
    if (auto const *home = getenv("HOME"); home != nullptr)
    {
        return optional<path> { path { home } / ".cache" / name };
    }
    return std::nullopt;
}

[[nodiscard]]
inline auto get_runtime_dir(GetEnvFn getenv, std::string_view const name) -> optional<path>
{
    if (auto const *xdg_runtime_dir = getenv("XDG_RUNTIME_DIR"); xdg_runtime_dir != nullptr)
    {
        return optional<path> { path { xdg_runtime_dir } / name };
    }
    auto const uid = ::getuid();
    return optional<path> { path { "/run/user" } / std::to_string(uid) / name };
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
