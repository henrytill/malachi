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

constexpr std::string to_string(Platform p)
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

} // namespace malachi::config
