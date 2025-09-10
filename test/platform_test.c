#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "platform.h"
#include "test.h"

void
test_platform_to_string(void)
{
	BEGIN_TEST();
	char const *platform_str = platform_to_string();
	TEST(platform_str != NULL);
	TEST(strlen(platform_str) > 0);
	printf("Platform: %s\n", platform_str);
	END_TEST();
}

char *
test_getenv(char const *name)
{
	if (strcmp(name, "HOME") == 0) {
		return "/home/test";
	}
	if (strcmp(name, "XDG_CONFIG_HOME") == 0) {
		return "/tmp/config";
	}
	if (strcmp(name, "APPDATA") == 0) {
		return "C:\\Users\\test\\AppData\\Roaming";
	}
	if (strcmp(name, "LOCALAPPDATA") == 0) {
		return "C:\\Users\\test\\AppData\\Local";
	}
	return NULL;
}

void
test_platform_get_config_dir(void)
{
	BEGIN_TEST();
	char *config_dir = platform_get_config_dir(test_getenv, "testapp");
	TEST(config_dir != NULL);
	printf("Config dir: %s\n", config_dir);
	free(config_dir);
	END_TEST();
}

void
test_platform_get_data_dir(void)
{
	BEGIN_TEST();
	char *data_dir = platform_get_data_dir(test_getenv, "testapp");
	TEST(data_dir != NULL);
	printf("Data dir: %s\n", data_dir);
	free(data_dir);
	END_TEST();
}

int
main(void)
{
	test_platform_to_string();
	test_platform_get_config_dir();
	test_platform_get_data_dir();
	return EXIT_SUCCESS;
}
