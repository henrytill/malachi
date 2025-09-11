#include <assert.h>
#include <stdlib.h>

#include "dat.h"
#include "fns.h"

char const *
platform_to_string(void)
{
	return "macOS";
}

static char *
platform_macos_get_application_support(platform_getenv_fn getenv, char const *name)
{
	assert(name != NULL);

	char const *home = getenv("HOME");
	if (home != NULL)
		return joinpath4(home, "Library", "Application Support", name);

	return NULL;
}

char *
platform_get_config_dir(platform_getenv_fn getenv, char const *name)
{
	return platform_macos_get_application_support(getenv, name);
}

char *
platform_get_data_dir(platform_getenv_fn getenv, char const *name)
{
	return platform_macos_get_application_support(getenv, name);
}
