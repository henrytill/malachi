#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "error.h"
#include "platform.h"
#include "test.h"

void test_config_init_with_defaults(void);
void test_platform_specific(void);

char *
getenv_empty(char const *name)
{
	(void)name;
	return NULL;
}

void
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

int
main(void)
{
	test_config_init_with_defaults();
	test_platform_specific();
	test_config_init_with_missing_dirs();
	return EXIT_SUCCESS;
}
