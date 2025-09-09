#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "error.h"
#include "platform.h"
#include "test.h"

#ifdef PLATFORM_LINUX
char *getenv_defaults(char const *name) {
    if (strcmp(name, "HOME") == 0) {
        return "/home/user";
    }
    return NULL;
}

void test_config_builder_with_defaults(void) {
    BEGIN_TEST();
    struct config_builder config_builder = {0};
    int rc = config_builder_init(&config_builder, getenv_defaults);
    TEST(rc == 0);
    config_builder_with_defaults(&config_builder);
    struct config config = {0};
    struct error error = {0};
    rc = config_builder_build(&config_builder, &config, &error);
    TEST(rc == 0);
    TEST(error.rc == rc);
    TEST(error.msg == NULL);
    TEST(strcmp(config.config_dir, "/home/user/.config/malachi") == 0);
    TEST(strcmp(config.data_dir, "/home/user/.local/share/malachi") == 0);
    config_finish(&config);
    END_TEST();
}

char *getenv_custom_xdg_dirs(char const *name) {
    if (strcmp(name, "XDG_CONFIG_HOME") == 0) {
        return "/tmp/config";
    }
    if (strcmp(name, "XDG_DATA_HOME") == 0) {
        return "/tmp/data";
    }
    return NULL;
}

void test_config_builder_with_custom_xdg_dirs(void) {
    BEGIN_TEST();
    struct config_builder config_builder = {0};
    int rc = config_builder_init(&config_builder, getenv_custom_xdg_dirs);
    TEST(rc == 0);
    config_builder_with_defaults(&config_builder);
    struct config config = {0};
    struct error error = {0};
    rc = config_builder_build(&config_builder, &config, &error);
    TEST(rc == 0);
    TEST(error.rc == rc);
    TEST(error.msg == NULL);
    TEST(strcmp(config.config_dir, "/tmp/config/malachi") == 0);
    TEST(strcmp(config.data_dir, "/tmp/data/malachi") == 0);
    config_finish(&config);
    END_TEST();
}
#endif

#ifdef PLATFORM_MACOS
char *getenv_defaults(const char *name) {
    if (strcmp(name, "HOME") == 0) {
        return "/Users/user";
    }
    return NULL;
}

void test_config_builder_with_defaults(void) {
    BEGIN_TEST();
    struct config_builder config_builder = {0};
    int rc = config_builder_init(&config_builder, getenv_defaults);
    TEST(rc == 0);
    config_builder_with_defaults(&config_builder);
    struct config config = {0};
    struct error error = {0};
    rc = config_builder_build(&config_builder, &config, &error);
    TEST(rc == 0);
    TEST(error.rc == rc);
    TEST(error.msg == NULL);
    TEST(strcmp(config.config_dir, "/Users/user/Library/Application Support/malachi") == 0);
    TEST(strcmp(config.data_dir, "/Users/user/Library/Application Support/malachi") == 0);
    config_finish(&config);
    END_TEST();
}
#endif

#ifdef PLATFORM_WINDOWS
char *getenv_defaults(const char *name) {
    if (strcmp(name, "APPDATA") == 0) {
        return "C:/Users/user/AppData/Roaming";
    }
    if (strcmp(name, "LOCALAPPDATA") == 0) {
        return "C:/Users/user/AppData/Local";
    }
    return NULL;
}

void test_config_builder_with_defaults(void) {
    BEGIN_TEST();
    struct config_builder config_builder = {0};
    int rc = config_builder_init(&config_builder, getenv_defaults);
    TEST(rc == 0);
    config_builder_with_defaults(&config_builder);
    struct config config = {0};
    struct error error = {0};
    rc = config_builder_build(&config_builder, &config, &error);
    TEST(rc == 0);
    TEST(error.rc == rc);
    TEST(error.msg == NULL);
    TEST(strcmp(config.config_dir, "C:/Users/user/AppData/Roaming/malachi") == 0);
    TEST(strcmp(config.data_dir, "C:/Users/user/AppData/Local/malachi") == 0);
    config_finish(&config);
    END_TEST();
}
#endif

char *getenv_empty(const char *name) {
    (void)name;
    return NULL;
}

void test_config_builder_with_missing_dirs(void) {
    BEGIN_TEST();
    struct config_builder config_builder = {0};
    int rc = config_builder_init(&config_builder, getenv_empty);
    TEST(rc == 0);
    config_builder_with_defaults(&config_builder);
    struct config config = {0};
    struct error error = {0};
    rc = config_builder_build(&config_builder, &config, &error);
    TEST(rc == -CONFIG_ERROR_MISSING_DIR);
    TEST(error.rc == -CONFIG_ERROR_MISSING_DIR);
    TEST(strcmp(error.msg, "maybe_config_dir is NULL") == 0);
    config_finish(&config);
    END_TEST();
}

int main(void) {
    test_config_builder_with_defaults();
#ifdef PLATFORM_LINUX
    test_config_builder_with_custom_xdg_dirs();
#endif
    test_config_builder_with_missing_dirs();
    return EXIT_SUCCESS;
}
