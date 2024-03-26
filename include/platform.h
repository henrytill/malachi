#ifndef MALACHI_INCLUDE_PLATFORM_H
#define MALACHI_INCLUDE_PLATFORM_H

#include <assert.h>
#include <stddef.h>

#include "path.h"

#if defined(_WIN32)
#define PLATFORM_WINDOWS
#elif defined(__APPLE__)
#define PLATFORM_MACOS
#elif defined(__linux__)
#define PLATFORM_LINUX
#else
#define PLATFORM_UNKNOWN
#endif

typedef char *platform_getenv_fn(const char *name);

static inline const char *platform_to_string(void)
{
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

static inline const char *platform_windows_get_app_data(platform_getenv_fn getenv, const char *name)
{
    assert(name != NULL);
    const char *app_data = getenv("APPDATA");
    if (app_data != NULL) {
        return joinpath2(app_data, name);
    }
    return NULL;
}

static inline const char *platform_windows_get_local_app_data(platform_getenv_fn getenv, const char *name)
{
    assert(name != NULL);
    const char *local_app_data = getenv("LOCALAPPDATA");
    if (local_app_data != NULL) {
        return joinpath2(local_app_data, name);
    }
    return NULL;
}

static inline const char *platform_macos_get_support_dir(platform_getenv_fn getenv, const char *name)
{
    assert(name != NULL);
    const char *home = getenv("HOME");
    if (home != NULL) {
        return joinpath4(home, "Library", "Application Support", name);
    }
    return NULL;
}

static inline const char *platform_xdg_get_config_home(platform_getenv_fn getenv, const char *name)
{
    assert(name != NULL);
    const char *xdg_config_home = getenv("XDG_CONFIG_HOME");
    if (xdg_config_home != NULL) {
        return joinpath2(xdg_config_home, name);
    }
    const char *home = getenv("HOME");
    if (home != NULL) {
        return joinpath3(home, ".config", name);
    }
    return NULL;
}

static inline const char *platform_xdg_get_data_home(platform_getenv_fn getenv, const char *name)
{
    assert(name != NULL);
    const char *xdg_data_home = getenv("XDG_DATA_HOME");
    if (xdg_data_home != NULL) {
        return joinpath2(xdg_data_home, name);
    }
    const char *home = getenv("HOME");
    if (home != NULL) {
        return joinpath4(home, ".local", "share", name);
    }
    return NULL;
}

static inline const char *platform_get_config_dir(platform_getenv_fn getenv, const char *name)
{
#if defined(PLATFORM_WINDOWS)
    return platform_windows_get_app_data(getenv, name);
#elif defined(PLATFORM_MACOS)
    return platform_macos_get_support_dir(getenv, name);
#else
    return platform_xdg_get_config_home(getenv, name);
#endif
}

static inline const char *platform_get_data_dir(platform_getenv_fn getenv, const char *name)
{
#if defined(PLATFORM_WINDOWS)
    return platform_windows_get_local_app_data(getenv, name);
#elif defined(PLATFORM_MACOS)
    return platform_macos_get_support_dir(getenv, name);
#else
    return platform_xdg_get_data_home(getenv, name);
#endif
}

#endif // MALACHI_INCLUDE_PLATFORM_H
