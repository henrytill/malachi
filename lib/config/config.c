#include "config.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "path.h"
#include "platform.h"

static const char *const MALACHI_DIR = "malachi";

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
    const char *maybe_config_dir;
    const char *maybe_data_dir;
};

struct config_builder *config_builder_create(void)
{
    struct config_builder *ret = calloc(1, sizeof(*ret));
    return ret;
};

#if defined(PLATFORM_WINDOWS)
static const char *get_windows_config_dir(config_getenv_fn getenv)
{
    const char *app_data = getenv("APPDATA");
    assert(app_data != NULL);
    return joinpath2(app_data, MALACHI_DIR);
}
#endif

#if defined(PLATFORM_WINDOWS)
static const char *get_windows_data_dir(config_getenv_fn getenv)
{
    const char *local_app_data = getenv("LOCALAPPDATA");
    assert(local_app_data != NULL);
    return joinpath2(local_app_data, MALACHI_DIR);
}
#endif

#if defined(PLATFORM_MACOS)
static const char *get_macos_support_dir(config_getenv_fn getenv)
{
    const char *home = getenv("HOME");
    assert(home != NULL);
    return joinpath4(home, "Library", "Application Support", MALACHI_DIR);
}
#endif

#if defined(PLATFORM_LINUX) || defined(PLATFORM_UNKNOWN)
static const char *get_xdg_config_home(config_getenv_fn getenv)
{
    const char *xdg_config_home = getenv("XDG_CONFIG_HOME");
    if (xdg_config_home != NULL) {
        return joinpath2(xdg_config_home, MALACHI_DIR);
    }
    const char *home = getenv("HOME");
    assert(home != NULL);
    return joinpath3(home, ".config", MALACHI_DIR);
}
#endif

#if defined(PLATFORM_LINUX) || defined(PLATFORM_UNKNOWN)
static const char *get_xdg_data_home(config_getenv_fn getenv)
{
    const char *xdg_data_home = getenv("XDG_DATA_HOME");
    if (xdg_data_home != NULL) {
        return joinpath2(xdg_data_home, MALACHI_DIR);
    }
    const char *home = getenv("HOME");
    assert(home != NULL);
    return joinpath4(home, ".local", "share", MALACHI_DIR);
}
#endif

#if defined(PLATFORM_WINDOWS)
void config_builder_with_defaults(struct config_builder *builder, config_getenv_fn getenv)
{
    builder->maybe_config_dir = get_windows_config_dir(getenv);
    builder->maybe_data_dir = get_windows_data_dir(getenv);
}
#elif defined(PLATFORM_MACOS)
void config_builder_with_defaults(struct config_builder *builder, config_getenv_fn getenv)
{
    const char *support_dir = get_macos_support_dir(getenv);
    builder->maybe_config_dir = support_dir;
    builder->maybe_data_dir = support_dir;
}
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_UNKNOWN)
void config_builder_with_defaults(struct config_builder *builder, config_getenv_fn getenv)
{
    builder->maybe_config_dir = get_xdg_config_home(getenv);
    builder->maybe_data_dir = get_xdg_data_home(getenv);
}
#endif

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
