#include <assert.h>
#include <stdlib.h>

#include "dat.h"
#include "fns.h"

char const *
platformstr(void)
{
	return "macOS";
}

static char *
appsupport(Getenvfn getenv, char const *name)
{
	assert(name != NULL);

	char const *homevar = getenv("HOME");
	if (homevar != NULL)
		return joinpath4(homevar, "Library", "Application Support", name);

	return NULL;
}

char *
getconfigdir(Getenvfn getenv, char const *name)
{
	return appsupport(getenv, name);
}

char *
getdatadir(Getenvfn getenv, char const *name)
{
	return appsupport(getenv, name);
}
