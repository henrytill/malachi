#ifndef MALACHI_INCLUDE_CONFIG_H
#define MALACHI_INCLUDE_CONFIG_H

struct error;

enum config_error {
    CONFIG_ERROR_MISSING_DIR = 1,
};

struct config {
    const char *config_dir;
    const char *data_dir;
};

void config_finish(struct config *config);

struct config_builder;

typedef char *platform_getenv_fn(const char *name);

struct config_builder *config_builder_create(platform_getenv_fn getenv);

void config_builder_with_defaults(struct config_builder *builder);

int config_builder_build(struct config_builder *builder, struct config *out, struct error *err);

#endif // MALACHI_INCLUDE_CONFIG_H
