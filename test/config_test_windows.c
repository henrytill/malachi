#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "test.h"

#include "config.h"

static const char *const NAME = "malachi";

char *getenv_defaults(const char *name)
{
    if (strcmp(name, "APPDATA") == 0) {
        return "C:\\Users\\user\\AppData\\Roaming";
    }
    if (strcmp(name, "LOCALAPPDATA") == 0) {
        return "C:\\Users\\user\\AppData\\Local";
    }
    return NULL;
}

void test_config_builder_with_defaults(void)
{
    BEGIN_TEST();
    struct config_builder *config_builder = config_builder_create(NAME);
    TEST(config_builder != NULL);
    config_builder_with_defaults(config_builder, getenv_defaults);
    struct config config = {0};
    struct error error = {0};
    const int rc = config_builder_build(config_builder, &config, &error);
    TEST(rc == 0);
    TEST(error.rc == rc);
    TEST(error.msg == NULL);
    TEST(strcmp(config.config_dir, "C:\\Users\\user\\AppData\\Roaming\\malachi") == 0);
    TEST(strcmp(config.data_dir, "C:\\Users\\user\\AppData\\Local\\malachi") == 0);
    config_finish(&config);
    END_TEST();
}

char *getenv_windows_local_app_data(const char *name)
{
    if (strcmp(name, "LOCALAPPDATA") == 0) {
        return "C:\\Users\\user\\AppData\\Local";
    }
    return NULL;
}

void test_config_builder_no_config_dir(void)
{
    BEGIN_TEST();
    struct config_builder *config_builder = config_builder_create(NAME);
    TEST(config_builder != NULL);
    config_builder_with_defaults(config_builder, getenv_windows_local_app_data);
    struct config config = {0};
    struct error error = {0};
    const int rc = config_builder_build(config_builder, &config, &error);
    TEST(rc == -CONFIG_ERROR_MISSING_DIR);
    TEST(error.rc == -CONFIG_ERROR_MISSING_DIR);
    TEST(strcmp(error.msg, "maybe_config_dir is NULL") == 0);
    config_finish(&config);
    END_TEST();
}

char *getenv_windows_app_data(const char *name)
{
    if (strcmp(name, "APPDATA") == 0) {
        return "C:\\Users\\user\\AppData\\Roaming";
    }
    return NULL;
}

void test_config_builder_no_data_dir(void)
{
    BEGIN_TEST();
    struct config_builder *config_builder = config_builder_create(NAME);
    TEST(config_builder != NULL);
    config_builder_with_defaults(config_builder, getenv_windows_app_data);
    struct config config = {0};
    struct error error = {0};
    const int rc = config_builder_build(config_builder, &config, &error);
    TEST(rc == -CONFIG_ERROR_MISSING_DIR);
    TEST(error.rc == -CONFIG_ERROR_MISSING_DIR);
    TEST(strcmp(error.msg, "maybe_data_dir is NULL") == 0);
    config_finish(&config);
    END_TEST();
}

int main(void)
{
    test_config_builder_with_defaults();
    test_config_builder_no_config_dir();
    test_config_builder_no_data_dir();
    return EXIT_SUCCESS;
}
