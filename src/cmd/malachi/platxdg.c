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

	char const *xdg = getenv("XDG_CONFIG_HOME");
	if(xdg != NULL)
		return joinpath2(xdg, name);

	char const *home = getenv("HOME");
	if(home != NULL)
		return joinpath3(home, ".config", name);

	return NULL;
}

static char *
xdgdatahome(Getenvfn getenv, char const *name)
{
	assert(name != NULL);

	char const *xdg = getenv("XDG_DATA_HOME");
	if(xdg != NULL)
		return joinpath2(xdg, name);

	char const *home = getenv("HOME");
	if(home != NULL)
		return joinpath4(home, ".local", "share", name);

	return NULL;
}

static char *
xdgcachehome(Getenvfn getenv, char const *name)
{
	assert(name != NULL);

	char const *xdg = getenv("XDG_CACHE_HOME");
	if(xdg != NULL)
		return joinpath2(xdg, name);

	char const *home = getenv("HOME");
	if(home != NULL)
		return joinpath3(home, ".cache", name);

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
