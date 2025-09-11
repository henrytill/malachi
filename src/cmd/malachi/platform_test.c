#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dat.h"
#include "fns.h"

#include "test.h"

static void
test_platform_to_string(void)
{
	BEGIN_TEST();
	char const *platform_str = platform_to_string();
	TEST(platform_str != NULL);
	TEST(strlen(platform_str) > 0);
	printf("Platform: %s\n", platform_str);
	END_TEST();
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

static void
test_platform_get_config_dir(void)
{
	BEGIN_TEST();
	char *config_dir = platform_get_config_dir(test_getenv, "testapp");
	TEST(config_dir != NULL);
	printf("Config dir: %s\n", config_dir);
	free(config_dir);
	END_TEST();
}

static void
test_platform_get_data_dir(void)
{
	BEGIN_TEST();
	char *data_dir = platform_get_data_dir(test_getenv, "testapp");
	TEST(data_dir != NULL);
	printf("Data dir: %s\n", data_dir);
	free(data_dir);
	END_TEST();
}

static int
platform_test_run(void)
{
	test_platform_to_string();
	test_platform_get_config_dir();
	test_platform_get_data_dir();
	return EXIT_SUCCESS;
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
