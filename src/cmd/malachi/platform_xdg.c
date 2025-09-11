#include <assert.h>
#include <stdlib.h>

#include "dat.h"
#include "fns.h"

char const *
platform_to_string(void)
{
	return "Linux";
}

static char *
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

static char *
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

char *
platform_get_config_dir(platform_getenv_fn getenv, char const *name)
{
	return platform_xdg_get_config_home(getenv, name);
}

char *
platform_get_data_dir(platform_getenv_fn getenv, char const *name)
{
	return platform_xdg_get_data_home(getenv, name);
}
