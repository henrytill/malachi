#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dat.h"
#include "fns.h"

#include "test.h"

static char *
getenv_defaults(char const *name)
{
	if (strcmp(name, "HOME") == 0)
		return "/home/user";

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
	TEST(strcmp(config.config_dir, "/home/user/.config/malachi") == 0);
	TEST(strcmp(config.data_dir, "/home/user/.local/share/malachi") == 0);
	config_finish(&config);
	END_TEST();
}

static char *
getenv_custom_xdg_dirs(char const *name)
{
	if (strcmp(name, "XDG_CONFIG_HOME") == 0)
		return "/tmp/config";

	if (strcmp(name, "XDG_DATA_HOME") == 0)
		return "/tmp/data";

	return NULL;
}

static void
test_config_init_with_custom_xdg_dirs(void)
{
	BEGIN_TEST();
	struct config config = {0};
	struct error error = {0};
	int rc = config_init(getenv_custom_xdg_dirs, &config, &error);
	TEST(rc == 0);
	TEST(error.rc == rc);
	TEST(error.msg == NULL);
	TEST(strcmp(config.config_dir, "/tmp/config/malachi") == 0);
	TEST(strcmp(config.data_dir, "/tmp/data/malachi") == 0);
	config_finish(&config);
	END_TEST();
}

void
test_platform_specific(void)
{
	test_config_init_with_custom_xdg_dirs();
}
