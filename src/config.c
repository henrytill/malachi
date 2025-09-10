#include "config.h"

#include <assert.h>
#include <stdlib.h>

#include "error.h"
#include "platform.h"

#define NAME "malachi"

#define MISSING_CONFIG_DIR_MSG "maybe_config_dir is NULL"
#define MISSING_DATA_DIR_MSG   "maybe_data_dir is NULL"

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

int
config_builder_init(struct config_builder *builder, platform_getenv_fn getenv)
{
	builder->getenv = getenv;
	return 0;
}

void
config_builder_with_defaults(struct config_builder *builder)
{
	builder->maybe_config_dir = platform_get_config_dir(builder->getenv, NAME);
	builder->maybe_data_dir = platform_get_data_dir(builder->getenv, NAME);
}

static void
config_builder_finish(struct config_builder *builder)
{
	if (builder->maybe_config_dir != NULL) {
		free(builder->maybe_config_dir);
		builder->maybe_config_dir = NULL;
	}
	if (builder->maybe_data_dir != NULL) {
		free(builder->maybe_data_dir);
		builder->maybe_data_dir = NULL;
	}
}

int
config_builder_build(struct config_builder *builder, struct config *out, struct error *err)
{
	int ret = 0;
	if (builder->maybe_config_dir == NULL) {
		ret = -CONFIG_ERROR_MISSING_DIR;
		err->rc = ret;
		err->msg = MISSING_CONFIG_DIR_MSG;
		config_builder_finish(builder);
		return ret;
	}
	if (builder->maybe_data_dir == NULL) {
		ret = -CONFIG_ERROR_MISSING_DIR;
		err->rc = ret;
		err->msg = MISSING_DATA_DIR_MSG;
		config_builder_finish(builder);
		return ret;
	}
	out->config_dir = builder->maybe_config_dir;
	out->data_dir = builder->maybe_data_dir;
	return ret;
}
