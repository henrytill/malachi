#pragma once

#include <cstdint>
#include <string>

namespace malachi::config {

enum class Platform : uint8_t {
  Windows,
  MacOS,
  Linux,
  Unknown,
};

constexpr auto get_platform() -> Platform {
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

constexpr auto to_string(Platform p) -> std::string {
  switch (p) {
  case Platform::Windows:
    return "Windows";
  case Platform::MacOS:
    return "MacOS";
  case Platform::Linux:
    return "Linux";
  case Platform::Unknown:
    [[fallthrough]];
  default:
    return "Unknown";
  };
}

} // namespace malachi::config
