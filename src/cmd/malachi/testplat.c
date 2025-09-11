#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dat.h"
#include "fns.h"

static int
testplatformstr(void)
{
	char const *plat = platformstr();
	if(plat == NULL) {
		eprintf("platformstr returned NULL\n");
		return -1;
	}
	if(strlen(plat) == 0) {
		eprintf("platformstr returned empty string\n");
		return -1;
	}
	return 0;
}

static char *
testgetenv(char const *name)
{
	if(strcmp(name, "HOME") == 0)
		return "/home/test";

	if(strcmp(name, "XDG_CONFIG_HOME") == 0)
		return "/tmp/config";

	return NULL;
}

static int
testconfigdir(void)
{
	char *configdir = getconfigdir(testgetenv, "testapp");
	if(configdir == NULL) {
		eprintf("getconfigdir returned NULL\n");
		return -1;
	}
	free(configdir);
	return 0;
}

static int
testdatadir(void)
{
	char *datadir = getdatadir(testgetenv, "testapp");
	if(datadir == NULL) {
		eprintf("getdatadir returned NULL\n");
		return -1;
	}
	free(datadir);
	return 0;
}

static int
run(void)
{
	int failures = 0;

	if(testplatformstr() != 0)
		failures++;
	if(testconfigdir() != 0)
		failures++;
	if(testdatadir() != 0)
		failures++;

	return failures;
}

static Test const test = {
	.name = "platform",
	.run = run,
};

__attribute__((constructor)) static void
init(void)
{
	testadd(&test);
}
