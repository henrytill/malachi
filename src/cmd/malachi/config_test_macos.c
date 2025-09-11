#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dat.h"
#include "fns.h"

static char *
getenv_defaults(char const *name)
{
	if (strcmp(name, "HOME") == 0)
		return "/Users/test";

	return NULL;
}

int
test_config_init_with_defaults(void)
{
	struct config config = {0};
	struct error error = {0};
	int rc = config_init(getenv_defaults, &config, &error);
	if (rc != 0) {
		eprintf("ERROR: config_init failed with rc=%d\n", rc);
		return -1;
	}
	if (error.rc != rc) {
		eprintf("ERROR: error.rc mismatch\n");
		return -1;
	}
	if (error.msg != NULL) {
		eprintf("ERROR: error.msg should be NULL\n");
		return -1;
	}
	if (strcmp(config.config_dir, "/Users/test/Library/Application Support/malachi") != 0) {
		eprintf("ERROR: config_dir mismatch: %s\n", config.config_dir);
		return -1;
	}
	if (strcmp(config.data_dir, "/Users/test/Library/Application Support/malachi") != 0) {
		eprintf("ERROR: data_dir mismatch: %s\n", config.data_dir);
		return -1;
	}
	config_finish(&config);
	return 0;
}

int
test_platform_specific(void)
{
	return 0;
}
