#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dat.h"
#include "fns.h"

int test_config_init_with_defaults(void);
int test_platform_specific(void);

static char *
getenv_empty(char const *name)
{
	(void)name;
	return NULL;
}

static int
test_config_init_with_missing_dirs(void)
{
	struct config config = {0};
	struct error error = {0};
	int rc = config_init(getenv_empty, &config, &error);
	if (rc != -CONFIG_ERROR_MISSING_DIR) {
		eprintf("ERROR: expected rc=%d, got rc=%d\n", -CONFIG_ERROR_MISSING_DIR, rc);
		return -1;
	}
	if (error.rc != -CONFIG_ERROR_MISSING_DIR) {
		eprintf("ERROR: expected error.rc=%d, got error.rc=%d\n", -CONFIG_ERROR_MISSING_DIR, error.rc);
		return -1;
	}
	if (strcmp(error.msg, "config_dir is NULL") != 0) {
		eprintf("ERROR: expected error.msg='config_dir is NULL', got '%s'\n", error.msg);
		return -1;
	}
	config_finish(&config);
	return 0;
}

static int
config_test_run(void)
{
	int failures = 0;

	if (test_config_init_with_defaults() != 0)
		failures++;
	if (test_platform_specific() != 0)
		failures++;
	if (test_config_init_with_missing_dirs() != 0)
		failures++;

	return failures;
}

static struct test_ops const config_test_ops = {
	.name = "config",
	.run = config_test_run,
};

__attribute__((constructor)) static void
config_test_init(void)
{
	test_register(&config_test_ops);
}
