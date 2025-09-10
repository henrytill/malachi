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

typedef char *platform_getenv_fn(char const *name);

int config_init(platform_getenv_fn getenv, struct config *config, struct error *err);

void config_finish(struct config *config);

#endif // MALACHI_INCLUDE_CONFIG_H
