#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dat.h"
#include "fns.h"

#include "test.h"

void test_config_init_with_defaults(void);
void test_platform_specific(void);

static char *
getenv_empty(char const *name)
{
	(void)name;
	return NULL;
}

static void
test_config_init_with_missing_dirs(void)
{
	BEGIN_TEST();
	struct config config = {0};
	struct error error = {0};
	int rc = config_init(getenv_empty, &config, &error);
	TEST(rc == -CONFIG_ERROR_MISSING_DIR);
	TEST(error.rc == -CONFIG_ERROR_MISSING_DIR);
	TEST(strcmp(error.msg, "config_dir is NULL") == 0);
	config_finish(&config);
	END_TEST();
}

static int
config_test_run(void)
{
	test_config_init_with_defaults();
	test_platform_specific();
	test_config_init_with_missing_dirs();
	return EXIT_SUCCESS;
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
