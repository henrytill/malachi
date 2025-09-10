#include "config.h"

#include <assert.h>
#include <stdlib.h>

#include "error.h"
#include "platform.h"

#define NAME "malachi"

int
config_init(platform_getenv_fn getenv, struct config *config, struct error *err)
{
	char *config_dir = platform_get_config_dir(getenv, NAME);
	char *data_dir = platform_get_data_dir(getenv, NAME);

	if (config_dir == NULL) {
		err->rc = -CONFIG_ERROR_MISSING_DIR;
		err->msg = "config_dir is NULL";
		free(data_dir);
		return -CONFIG_ERROR_MISSING_DIR;
	}

	if (data_dir == NULL) {
		err->rc = -CONFIG_ERROR_MISSING_DIR;
		err->msg = "data_dir is NULL";
		free(config_dir);
		return -CONFIG_ERROR_MISSING_DIR;
	}

	config->config_dir = config_dir;
	config->data_dir = data_dir;
	return 0;
}

void
config_finish(struct config *config)
{
	if (config->config_dir == config->data_dir) {
		free(config->config_dir);
	} else {
		free(config->config_dir);
		free(config->data_dir);
	}
}
