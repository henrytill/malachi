#pragma once

#if defined(_WIN32)
#define PLATFORM_WINDOWS
#elif defined(__APPLE__)
#define PLATFORM_MACOS
#elif defined(__linux__)
#define PLATFORM_LINUX
#else
#define PLATFORM_UNKNOWN
#endif

static inline const char *platform_to_string(void) {
#if defined(PLATFORM_WINDOWS)
    return "Windows";
#elif defined(PLATFORM_MACOS)
    return "macOS";
#elif defined(PLATFORM_LINUX)
    return "Linux";
#elif defined(PLATFORM_UNKNOWN)
    return "Unknown";
#endif
}
