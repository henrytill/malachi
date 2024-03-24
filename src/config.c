#include "config.h"

#include <assert.h>
#include <stdlib.h>

#include "platform.h"

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
    ret->name = name;
    return ret;
};

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

int config_builder_build(struct config_builder *builder, struct config *out)
{
    int ret = 0;
    if (builder->maybe_config_dir == NULL) {
        ret = CONFIG_BUILDER_ERROR_MISSING_CONFIG_DIR;
        config_builder_finish(builder);
        goto out_free_builder;
    }
    if (builder->maybe_data_dir == NULL) {
        ret = CONFIG_BUILDER_ERROR_MISSING_DATA_DIR;
        config_builder_finish(builder);
        goto out_free_builder;
    }
    out->config_dir = builder->maybe_config_dir;
    out->data_dir = builder->maybe_data_dir;
out_free_builder:
    free(builder);
    return ret;
};

static const char *const CONFIG_BUILDER_ERROR_STRINGS[] = {
#define X(tag, value, description) [-(CONFIG_BUILDER_ERROR_##tag)] = (description),
    CONFIG_BUILDER_ERROR_VARIANTS
#undef X
};

const char *config_builder_error_to_string(const int rc)
{
    extern const char *const CONFIG_BUILDER_ERROR_STRINGS[];
    if (rc >= 0 || rc <= CONFIG_BUILDER_ERROR_MIN) { return NULL; }
    return CONFIG_BUILDER_ERROR_STRINGS[-rc];
}
