#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "test.h"

#include "config.h"
#include "platform.h"

void test_platform_to_string(void) {
  BEGIN_TEST();
  TEST(strcmp(platform_to_string(PLATFORM_UNKNOWN), "Unknown") == 0);
  TEST(strcmp(platform_to_string(PLATFORM_WINDOWS), "Windows") == 0);
  TEST(strcmp(platform_to_string(PLATFORM_MACOS), "MacOS") == 0);
  TEST(strcmp(platform_to_string(PLATFORM_LINUX), "Linux") == 0);
  END_TEST();
}

char *getenv_mock_windows_defaults(const char *name) {
  if (strcmp(name, "APPDATA") == 0) {
    return "C:\\Users\\user\\AppData\\Roaming";
  }
  if (strcmp(name, "LOCALAPPDATA") == 0) {
    return "C:\\Users\\user\\AppData\\Local";
  }
  return NULL;
}

void test_config_builder_with_windows_defaults(void) {
  BEGIN_TEST();
  struct config config = {0};
  struct config_builder *config_builder = config_builder_create();
  config_builder_with_defaults(config_builder, PLATFORM_WINDOWS, getenv_mock_windows_defaults);
  config_builder_build(config_builder, &config);
  TEST(strcmp(config.config_dir, "C:\\Users\\user\\AppData\\Roaming\\malachi") == 0);
  TEST(strcmp(config.data_dir, "C:\\Users\\user\\AppData\\Local\\malachi") == 0);
  config_finish(&config);
  END_TEST();
}

char *getenv_mock_macos_defaults(const char *name) {
  if (strcmp(name, "HOME") == 0) {
    return "/Users/user";
  }
  return NULL;
}

void test_config_builder_with_macos_defaults(void) {
  BEGIN_TEST();
  struct config config = {0};
  struct config_builder *config_builder = config_builder_create();
  config_builder_with_defaults(config_builder, PLATFORM_MACOS, getenv_mock_macos_defaults);
  config_builder_build(config_builder, &config);
  TEST(strcmp(config.config_dir, "/Users/user/Library/Application Support/malachi") == 0);
  TEST(strcmp(config.data_dir, "/Users/user/Library/Application Support/malachi") == 0);
  config_finish(&config);
  END_TEST();
}

char *getenv_mock_unix_defaults(const char *name) {
  if (strcmp(name, "HOME") == 0) {
    return "/home/user";
  }
  return NULL;
}

void test_config_builder_with_linux_defaults(void) {
  BEGIN_TEST();
  struct config config = {0};
  struct config_builder *config_builder = config_builder_create();
  config_builder_with_defaults(config_builder, PLATFORM_LINUX, getenv_mock_unix_defaults);
  config_builder_build(config_builder, &config);
  TEST(strcmp(config.config_dir, "/home/user/.config/malachi") == 0);
  TEST(strcmp(config.data_dir, "/home/user/.local/share/malachi") == 0);
  config_finish(&config);
  END_TEST();
}

void test_config_builder_with_unknown_defaults(void) {
  BEGIN_TEST();
  struct config config = {0};
  struct config_builder *config_builder = config_builder_create();
  config_builder_with_defaults(config_builder, PLATFORM_UNKNOWN, getenv_mock_unix_defaults);
  config_builder_build(config_builder, &config);
  TEST(strcmp(config.config_dir, "/home/user/.config/malachi") == 0);
  TEST(strcmp(config.data_dir, "/home/user/.local/share/malachi") == 0);
  config_finish(&config);
  END_TEST();
}

char *getenv_mock_custom_xdg_dirs(const char *name) {
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
  struct config config = {0};
  struct config_builder *config_builder = config_builder_create();
  config_builder_with_defaults(config_builder, PLATFORM_LINUX, getenv_mock_custom_xdg_dirs);
  config_builder_build(config_builder, &config);
  TEST(strcmp(config.config_dir, "/tmp/config/malachi") == 0);
  TEST(strcmp(config.data_dir, "/tmp/data/malachi") == 0);
  config_finish(&config);
  END_TEST();
}

int main(void) {
  test_platform_to_string();
  test_config_builder_with_windows_defaults();
  test_config_builder_with_macos_defaults();
  test_config_builder_with_linux_defaults();
  test_config_builder_with_unknown_defaults();
  test_config_builder_with_custom_xdg_dirs();
  return EXIT_SUCCESS;
}