#pragma once

enum platform {
  PLATFORM_UNKNOWN = 0,
  PLATFORM_WINDOWS = 1,
  PLATFORM_MACOS = 2,
  PLATFORM_LINUX = 3,
};

static inline enum platform platform_get(void) {
#if defined(_WIN32)
  return PLATFORM_WINDOWS;
#elif defined(__APPLE__)
  return PLATFORM_MACOS;
#elif defined(__linux__)
  return PLATFORM_LINUX;
#else
  return PLATFORM_UNKNOWN;
#endif
}

static inline const char *platform_to_string(enum platform p) {
  switch (p) {
  case PLATFORM_WINDOWS:
    return "Windows";
  case PLATFORM_MACOS:
    return "MacOS";
  case PLATFORM_LINUX:
    return "Linux";
  case PLATFORM_UNKNOWN:
    __attribute__((fallthrough));
  default:
    return "Unknown";
  }
}
