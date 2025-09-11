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

	if (configdir == NULL) {
		err->rc = -EMISSINGDIR;
		err->msg = "configdir is NULL";
		free(datadir);
		return -EMISSINGDIR;
	}

	if (datadir == NULL) {
		err->rc = -EMISSINGDIR;
		err->msg = "datadir is NULL";
		free(configdir);
		return -EMISSINGDIR;
	}

	config->configdir = configdir;
	config->datadir = datadir;
	return 0;
}

void
configfree(Config *config)
{
	if (config->configdir == config->datadir) {
		free(config->configdir);
	} else {
		free(config->configdir);
		free(config->datadir);
	}
}
