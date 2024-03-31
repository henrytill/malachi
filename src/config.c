#include "config.h"

#include <assert.h>
#include <stdlib.h>

#include "error.h"
#include "platform.h"

#define MISSING_CONFIG_DIR_MSG "maybe_config_dir is NULL"
#define MISSING_DATA_DIR_MSG   "maybe_data_dir is NULL"

void config_finish(struct config *config)
{
    if (config->config_dir == config->data_dir) {
        free((char *)config->config_dir);
    } else {
        free((char *)config->config_dir);
        free((char *)config->data_dir);
    }
}

struct config_builder {
    const char *name;
    const char *maybe_config_dir;
    const char *maybe_data_dir;
};

struct config_builder *config_builder_create(const char *name)
{
    assert(name != NULL);
    struct config_builder *ret = calloc(1, sizeof(*ret));
    if (ret != NULL) { ret->name = name; }
    return ret;
}

void config_builder_with_defaults(struct config_builder *builder, platform_getenv_fn getenv)
{
    builder->maybe_config_dir = platform_get_config_dir(getenv, builder->name);
    builder->maybe_data_dir = platform_get_data_dir(getenv, builder->name);
}

static void config_builder_finish(struct config_builder *builder)
{
    if (builder->maybe_config_dir != NULL) {
        free((char *)builder->maybe_config_dir);
        builder->maybe_config_dir = NULL;
    }
    if (builder->maybe_data_dir != NULL) {
        free((char *)builder->maybe_data_dir);
        builder->maybe_data_dir = NULL;
    }
}

int config_builder_build(struct config_builder *builder, struct config *out, struct error *err)
{
    int ret = 0;
    if (builder->maybe_config_dir == NULL) {
        ret = -CONFIG_ERROR_MISSING_DIR;
        err->rc = ret;
        err->msg = MISSING_CONFIG_DIR_MSG;
        config_builder_finish(builder);
        goto out_free_builder;
    }
    if (builder->maybe_data_dir == NULL) {
        ret = -CONFIG_ERROR_MISSING_DIR;
        err->rc = ret;
        err->msg = MISSING_DATA_DIR_MSG;
        config_builder_finish(builder);
        goto out_free_builder;
    }
    out->config_dir = builder->maybe_config_dir;
    out->data_dir = builder->maybe_data_dir;
out_free_builder:
    free(builder);
    return ret;
}
