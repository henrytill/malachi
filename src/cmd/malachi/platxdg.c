#include <assert.h>
#include <stdlib.h>

#include "dat.h"
#include "fns.h"

char const *
platformstr(void)
{
	return "Linux";
}

static char *
xdgconfighome(Getenvfn getenv, char const *name)
{
	assert(name != NULL);

	char const *xdgvar = getenv("XDG_CONFIG_HOME");
	if (xdgvar != NULL)
		return joinpath2(xdgvar, name);

	char const *homevar = getenv("HOME");
	if (homevar != NULL)
		return joinpath3(homevar, ".config", name);

	return NULL;
}

static char *
xdgdatahome(Getenvfn getenv, char const *name)
{
	assert(name != NULL);

	char const *xdgvar = getenv("XDG_DATA_HOME");
	if (xdgvar != NULL)
		return joinpath2(xdgvar, name);

	char const *homevar = getenv("HOME");
	if (homevar != NULL)
		return joinpath4(homevar, ".local", "share", name);

	return NULL;
}

static char *
xdgcachehome(Getenvfn getenv, char const *name)
{
	assert(name != NULL);

	char const *xdgvar = getenv("XDG_CACHE_HOME");
	if (xdgvar != NULL)
		return joinpath2(xdgvar, name);

	char const *homevar = getenv("HOME");
	if (homevar != NULL)
		return joinpath3(homevar, ".cache", name);

	return NULL;
}

char *
getconfigdir(Getenvfn getenv, char const *name)
{
	return xdgconfighome(getenv, name);
}

char *
getdatadir(Getenvfn getenv, char const *name)
{
	return xdgdatahome(getenv, name);
}

char *
getcachedir(Getenvfn getenv, char const *name)
{
	return xdgcachehome(getenv, name);
}
