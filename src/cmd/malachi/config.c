#include <stdlib.h>

#include "malachi.h"

int
configinit(Getenvfn getenv, Config *config, Error *err)
{
	char *configdir = getconfigdir(getenv, appname);
	char *datadir = getdatadir(getenv, appname);
	char *cachedir = getcachedir(getenv, appname);
	char *runtimedir = getruntimedir(getenv, appname);

	if(configdir == NULL) {
		err->rc = -Emissingdir;
		err->msg = "configdir is NULL";
		free(datadir);
		free(cachedir);
		free(runtimedir);
		return -Emissingdir;
	}

	if(datadir == NULL) {
		err->rc = -Emissingdir;
		err->msg = "datadir is NULL";
		free(configdir);
		free(cachedir);
		free(runtimedir);
		return -Emissingdir;
	}

	if(cachedir == NULL) {
		err->rc = -Emissingdir;
		err->msg = "cachedir is NULL";
		free(configdir);
		free(datadir);
		free(runtimedir);
		return -Emissingdir;
	}

	if(runtimedir == NULL) {
		err->rc = -Emissingdir;
		err->msg = "runtimedir is NULL";
		free(configdir);
		free(datadir);
		free(cachedir);
		return -Emissingdir;
	}

	config->configdir = configdir;
	config->datadir = datadir;
	config->cachedir = cachedir;
	config->runtimedir = runtimedir;
	return 0;
}

void
configfree(Config *config)
{
	void *ptrs[] = {config->configdir, config->datadir, config->cachedir, config->runtimedir};
	int freed[] = {0, 0, 0, 0};

	for(int i = 0; i < 4; i++) {
		if(freed[i] || ptrs[i] == NULL)
			continue;

		for(int j = i + 1; j < 4; j++) {
			if(ptrs[i] == ptrs[j]) {
				freed[j] = 1;
			}
		}

		free(ptrs[i]);
		freed[i] = 1;
	}
}
