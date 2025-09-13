#include <stdlib.h>

#include "malachi.h"

int
configinit(Getenvfn getenv, Config *config, Error *err)
{
	char *configdir = getconfigdir(getenv, appname);
	char *datadir = getdatadir(getenv, appname);
	char *cachedir = getcachedir(getenv, appname);

	if(configdir == NULL) {
		err->rc = -Emissingdir;
		err->msg = "configdir is NULL";
		free(datadir);
		free(cachedir);
		return -Emissingdir;
	}

	if(datadir == NULL) {
		err->rc = -Emissingdir;
		err->msg = "datadir is NULL";
		free(configdir);
		free(cachedir);
		return -Emissingdir;
	}

	if(cachedir == NULL) {
		err->rc = -Emissingdir;
		err->msg = "cachedir is NULL";
		free(configdir);
		free(datadir);
		return -Emissingdir;
	}

	config->configdir = configdir;
	config->datadir = datadir;
	config->cachedir = cachedir;
	return 0;
}

void
configfree(Config *config)
{
	if(config->configdir == config->datadir && config->configdir == config->cachedir) {
		free(config->configdir);
	} else {
		free(config->configdir);
		free(config->datadir);
		free(config->cachedir);
	}
}
