#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "malachi.h"

char *
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

static char *
xdgruntimedir(Getenvfn getenv, char const *name)
{
	assert(name != NULL);

	{
		char const *xdg = getenv("XDG_RUNTIME_DIR");
		if(xdg != NULL)
			return joinpath2(xdg, name);
	}

	{
		uid_t uid = getuid();

		{
			size_t len = (size_t)snprintf(NULL, 0, "/run/user/%u", uid);
			char *runuserpath = calloc(++len, sizeof(*runuserpath));
			if(runuserpath == NULL)
				return NULL;

			(void)snprintf(runuserpath, len, "/run/user/%u", uid);

			struct stat st;
			if(stat(runuserpath, &st) == 0 && S_ISDIR(st.st_mode) && (st.st_mode & S_IWUSR)) {
				char *result = joinpath2(runuserpath, name);
				free(runuserpath);
				return result;
			}

			free(runuserpath);
		}

		{
			size_t len = (size_t)snprintf(NULL, 0, "/tmp/%s-%u", name, uid);
			char *tmpname = calloc(++len, sizeof(*tmpname));
			if(tmpname == NULL)
				return NULL;

			(void)snprintf(tmpname, len, "/tmp/%s-%u", name, uid);

			return tmpname;
		}
	}
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

char *
getruntimedir(Getenvfn getenv, char const *name)
{
	return xdgruntimedir(getenv, name);
}
