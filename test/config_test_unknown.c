#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "test.h"

#include "config.h"

char *getenv_mock_defaults(const char *name)
{
    if (strcmp(name, "HOME") == 0) {
        return "/home/user";
    }
    return NULL;
}

void test_config_builder_with_defaults(void)
{
    BEGIN_TEST();
    struct config_builder *config_builder = config_builder_create();
    TEST(config_builder != NULL);
    config_builder_with_defaults(config_builder, getenv_mock_defaults);
    struct config config = {0};
    const int rc = config_builder_build(config_builder, &config);
    TEST(rc == 0);
    TEST(strcmp(config.config_dir, "/home/user/.config/malachi") == 0);
    TEST(strcmp(config.data_dir, "/home/user/.local/share/malachi") == 0);
    config_finish(&config);
    END_TEST();
}

char *getenv_mock_custom_xdg_dirs(const char *name)
{
    if (strcmp(name, "XDG_CONFIG_HOME") == 0) {
        return "/tmp/config";
    }
    if (strcmp(name, "XDG_DATA_HOME") == 0) {
        return "/tmp/data";
    }
    return NULL;
}

void test_config_builder_with_custom_xdg_dirs(void)
{
    BEGIN_TEST();
    struct config_builder *config_builder = config_builder_create();
    TEST(config_builder != NULL);
    config_builder_with_defaults(config_builder, getenv_mock_custom_xdg_dirs);
    struct config config = {0};
    const int rc = config_builder_build(config_builder, &config);
    TEST(rc == 0);
    TEST(strcmp(config.config_dir, "/tmp/config/malachi") == 0);
    TEST(strcmp(config.data_dir, "/tmp/data/malachi") == 0);
    config_finish(&config);
    END_TEST();
}

int main(void)
{
    test_config_builder_with_defaults();
    test_config_builder_with_custom_xdg_dirs();
    return EXIT_SUCCESS;
}
