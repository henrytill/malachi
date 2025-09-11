#include <assert.h>
#include <stdlib.h>

#include "dat.h"
#include "fns.h"

static char const *const name = "malachi";

int
configinit(Getenvfn getenv, Config *config, Error *err)
{
	char *configdir = getconfigdir(getenv, name);
	char *datadir = getdatadir(getenv, name);
	char *cachedir = getcachedir(getenv, name);

	if (configdir == NULL) {
		err->rc = -EMISSINGDIR;
		err->msg = "configdir is NULL";
		free(datadir);
		free(cachedir);
		return -EMISSINGDIR;
	}

	if (datadir == NULL) {
		err->rc = -EMISSINGDIR;
		err->msg = "datadir is NULL";
		free(configdir);
		free(cachedir);
		return -EMISSINGDIR;
	}

	if (cachedir == NULL) {
		err->rc = -EMISSINGDIR;
		err->msg = "cachedir is NULL";
		free(configdir);
		free(datadir);
		return -EMISSINGDIR;
	}

	config->configdir = configdir;
	config->datadir = datadir;
	config->cachedir = cachedir;
	return 0;
}

void
configfree(Config *config)
{
	if (config->configdir == config->datadir && config->configdir == config->cachedir) {
		free(config->configdir);
	} else {
		free(config->configdir);
		free(config->datadir);
		free(config->cachedir);
	}
}
