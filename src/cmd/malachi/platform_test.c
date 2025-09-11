#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dat.h"
#include "fns.h"

static int
test_platform_to_string(void)
{
	char const *platform_str = platform_to_string();
	if (platform_str == NULL) {
		eprintf("ERROR: platform_to_string returned NULL\n");
		return -1;
	}
	if (strlen(platform_str) == 0) {
		eprintf("ERROR: platform_to_string returned empty string\n");
		return -1;
	}
	return 0;
}

static char *
test_getenv(char const *name)
{
	if (strcmp(name, "HOME") == 0)
		return "/home/test";

	if (strcmp(name, "XDG_CONFIG_HOME") == 0)
		return "/tmp/config";

	return NULL;
}

static int
test_platform_get_config_dir(void)
{
	char *config_dir = platform_get_config_dir(test_getenv, "testapp");
	if (config_dir == NULL) {
		eprintf("ERROR: platform_get_config_dir returned NULL\n");
		return -1;
	}
	free(config_dir);
	return 0;
}

static int
test_platform_get_data_dir(void)
{
	char *data_dir = platform_get_data_dir(test_getenv, "testapp");
	if (data_dir == NULL) {
		eprintf("ERROR: platform_get_data_dir returned NULL\n");
		return -1;
	}
	free(data_dir);
	return 0;
}

static int
platform_test_run(void)
{
	int failures = 0;

	if (test_platform_to_string() != 0)
		failures++;
	if (test_platform_get_config_dir() != 0)
		failures++;
	if (test_platform_get_data_dir() != 0)
		failures++;

	return failures;
}

static struct test_ops const platform_test_ops = {
	.name = "platform",
	.run = platform_test_run,
};

__attribute__((constructor)) static void
platform_test_init(void)
{
	test_register(&platform_test_ops);
}
