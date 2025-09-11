#pragma once

struct error {
	int rc;
	char const *msg;
};

typedef char *platform_getenv_fn(char const *name);

enum config_error {
	CONFIG_ERROR_MISSING_DIR = 1,
};

struct config {
	char *config_dir;
	char *data_dir;
};

struct test_ops {
	char const *name;
	int (*run)(void);
};
