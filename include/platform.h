#pragma once

#include <cassert>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <optional>

namespace platform {

using std::filesystem::path;

using GetEnvFn = std::function<char *(const char *)>;

enum class Platform : uint8_t {
  Windows,
  MacOS,
  Linux,
  Unknown,
};

[[nodiscard]] constexpr auto get_platform() -> Platform {
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

[[nodiscard]] constexpr auto to_string_view(const Platform platform) -> std::string_view {
  switch (platform) {
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

[[nodiscard]] inline auto get_app_data(GetEnvFn getenv, const path name) noexcept -> std::optional<path> {
  auto app_data = std::unique_ptr<char>{getenv("APPDATA")};
  if (app_data != nullptr) {
    const auto config_dir = path{app_data.release()};
    return std::optional<path>{config_dir / name};
  }
  return std::nullopt;
}

[[nodiscard]] inline auto get_local_app_data(GetEnvFn getenv, const path name) noexcept -> std::optional<path> {
  auto local_app_data = std::unique_ptr<char>{getenv("LOCALAPPDATA")};
  if (local_app_data != nullptr) {
    const auto data_dir = path{local_app_data.release()};
    return std::optional<path>{data_dir / name};
  }
  return std::nullopt;
}

} // namespace windows

namespace mac_os {

[[nodiscard]] inline auto get_application_support(GetEnvFn getenv, const path name) noexcept -> std::optional<path> {
  auto home = std::unique_ptr<char>{getenv("HOME")};
  if (home != nullptr) {
    const auto home_dir = path{home.release()};
    return std::optional<path>{home_dir / "Library" / "Application Support" / name};
  }
  return std::nullopt;
}

} // namespace mac_os

namespace xdg {

[[nodiscard]] inline auto get_config_home(GetEnvFn getenv, const path name) noexcept -> std::optional<path> {
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

[[nodiscard]] inline auto get_data_home(GetEnvFn getenv, const path name) noexcept -> std::optional<path> {
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
[[nodiscard]] auto get_config_dir(GetEnvFn getenv, const path name) noexcept -> std::optional<path> {
  if constexpr (p == Platform::Windows) {
    return windows::get_app_data(getenv, name);
  } else if constexpr (p == Platform::MacOS) {
    return mac_os::get_application_support(getenv, name);
  } else {
    return xdg::get_config_home(getenv, name);
  }
}

template <Platform p = get_platform()>
[[nodiscard]] auto get_data_dir(GetEnvFn getenv, const path name) noexcept -> std::optional<path> {
  if constexpr (p == Platform::Windows) {
    return windows::get_local_app_data(getenv, name);
  } else if constexpr (p == Platform::MacOS) {
    return mac_os::get_application_support(getenv, name);
  } else {
    return xdg::get_data_home(getenv, name);
  }
}

} // namespace platform
