#ifndef MALACHI_INCLUDE_CONFIG_H
#define MALACHI_INCLUDE_CONFIG_H

struct error;

enum config_error {
    CONFIG_ERROR_MISSING_DIR = 1,
};

struct config {
    char *config_dir;
    char *data_dir;
};

void config_finish(struct config *config);

typedef char *platform_getenv_fn(char const *name);

struct config_builder {
    platform_getenv_fn *getenv;
    char *maybe_config_dir;
    char *maybe_data_dir;
};

int config_builder_init(struct config_builder *builder, platform_getenv_fn getenv);

void config_builder_with_defaults(struct config_builder *builder);

int config_builder_build(struct config_builder *builder, struct config *out, struct error *err);

#endif // MALACHI_INCLUDE_CONFIG_H
