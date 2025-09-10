#ifndef MALACHI_INCLUDE_PLATFORM_H
#define MALACHI_INCLUDE_PLATFORM_H

#include <assert.h>
#include <stddef.h>

#include "path.h"

#if defined(__APPLE__)
#	define PLATFORM_MACOS
#elif defined(__linux__)
#	define PLATFORM_LINUX
#else
#	define PLATFORM_UNKNOWN
#endif

typedef char *platform_getenv_fn(char const *name);

static inline char const *
platform_to_string(void)
{
#if defined(PLATFORM_MACOS)
	return "macOS";
#elif defined(PLATFORM_LINUX)
	return "Linux";
#elif defined(PLATFORM_UNKNOWN)
	return "Unknown";
#endif
}

static inline char *
platform_macos_get_application_support(platform_getenv_fn getenv, char const *name)
{
	assert(name != NULL);

	char const *home = getenv("HOME");
	if (home != NULL)
		return joinpath4(home, "Library", "Application Support", name);

	return NULL;
}

static inline char *
platform_xdg_get_config_home(platform_getenv_fn getenv, char const *name)
{
	assert(name != NULL);

	char const *xdg_config_home = getenv("XDG_CONFIG_HOME");
	if (xdg_config_home != NULL)
		return joinpath2(xdg_config_home, name);

	char const *home = getenv("HOME");
	if (home != NULL)
		return joinpath3(home, ".config", name);

	return NULL;
}

static inline char *
platform_xdg_get_data_home(platform_getenv_fn getenv, char const *name)
{
	assert(name != NULL);

	char const *xdg_data_home = getenv("XDG_DATA_HOME");
	if (xdg_data_home != NULL)
		return joinpath2(xdg_data_home, name);

	char const *home = getenv("HOME");
	if (home != NULL)
		return joinpath4(home, ".local", "share", name);

	return NULL;
}

static inline char *
platform_get_config_dir(platform_getenv_fn getenv, char const *name)
{
#if defined(PLATFORM_MACOS)
	return platform_macos_get_application_support(getenv, name);
#else
	return platform_xdg_get_config_home(getenv, name);
#endif
}

static inline char *
platform_get_data_dir(platform_getenv_fn getenv, char const *name)
{
#if defined(PLATFORM_MACOS)
	return platform_macos_get_application_support(getenv, name);
#else
	return platform_xdg_get_data_home(getenv, name);
#endif
}

#endif // MALACHI_INCLUDE_PLATFORM_H
