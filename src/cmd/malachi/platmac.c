#include <assert.h>
#include <stdlib.h>

#include "malachi.h"

char *
platformstr(void)
{
	return "macOS";
}

static char *
appsupport(Getenvfn getenv, char const *name)
{
	assert(name != NULL);

	char const *home = getenv("HOME");
	if(home != NULL)
		return joinpath4(home, "Library", "Application Support", name);

	return NULL;
}

static char *
caches(Getenvfn getenv, char const *name)
{
	assert(name != NULL);

	char const *home = getenv("HOME");
	if(home != NULL)
		return joinpath4(home, "Library", "Caches", name);

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

char *
getcachedir(Getenvfn getenv, char const *name)
{
	return caches(getenv, name);
}
