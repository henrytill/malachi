#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dat.h"
#include "fns.h"

static char *
getenv_defaults(char const *name)
{
	if (strcmp(name, "HOME") == 0)
		return "/home/user";

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
	if (strcmp(config.config_dir, "/home/user/.config/malachi") != 0) {
		eprintf("ERROR: config_dir mismatch: %s\n", config.config_dir);
		return -1;
	}
	if (strcmp(config.data_dir, "/home/user/.local/share/malachi") != 0) {
		eprintf("ERROR: data_dir mismatch: %s\n", config.data_dir);
		return -1;
	}
	config_finish(&config);
	return 0;
}

static char *
getenv_custom_xdg_dirs(char const *name)
{
	if (strcmp(name, "XDG_CONFIG_HOME") == 0)
		return "/tmp/config";

	if (strcmp(name, "XDG_DATA_HOME") == 0)
		return "/tmp/data";

	return NULL;
}

static int
test_config_init_with_custom_xdg_dirs(void)
{
	struct config config = {0};
	struct error error = {0};
	int rc = config_init(getenv_custom_xdg_dirs, &config, &error);
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
	if (strcmp(config.config_dir, "/tmp/config/malachi") != 0) {
		eprintf("ERROR: config_dir mismatch: %s\n", config.config_dir);
		return -1;
	}
	if (strcmp(config.data_dir, "/tmp/data/malachi") != 0) {
		eprintf("ERROR: data_dir mismatch: %s\n", config.data_dir);
		return -1;
	}
	config_finish(&config);
	return 0;
}

int
test_platform_specific(void)
{
	return test_config_init_with_custom_xdg_dirs();
}
