#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "error.h"
#include "platform.h"
#include "test.h"

char *
getenv_defaults(char const *name)
{
	if (strcmp(name, "HOME") == 0)
		return "/Users/user";

	return NULL;
}

void
test_config_init_with_defaults(void)
{
	BEGIN_TEST();
	struct config config = {0};
	struct error error = {0};
	int rc = config_init(getenv_defaults, &config, &error);
	TEST(rc == 0);
	TEST(error.rc == rc);
	TEST(error.msg == NULL);
	TEST(strcmp(config.config_dir, "/Users/user/Library/Application Support/malachi") == 0);
	TEST(strcmp(config.data_dir, "/Users/user/Library/Application Support/malachi") == 0);
	config_finish(&config);
	END_TEST();
}

void
test_platform_specific(void)
{
}
