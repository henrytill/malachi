#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "platform.h"

#define TEST(e)                                                   \
  if (!(e)) {                                                     \
    (void)fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #e); \
    exit(EXIT_FAILURE);                                           \
  }

void test_platform_to_string(void) {
  printf("%s:%s: Starting...\n", __FILE__, __func__);
  TEST(strcmp(platform_to_string(PLATFORM_UNKNOWN), "Unknown") == 0);
  TEST(strcmp(platform_to_string(PLATFORM_WINDOWS), "Windows") == 0);
  TEST(strcmp(platform_to_string(PLATFORM_MACOS), "MacOS") == 0);
  TEST(strcmp(platform_to_string(PLATFORM_LINUX), "Linux") == 0);
  printf("%s:%s: Passed!\n", __FILE__, __func__);
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
  printf("%s:%s: Starting...\n", __FILE__, __func__);
  struct config config = {};
  struct config_builder *config_builder = config_builder_create();
  config_builder_with_defaults(config_builder, PLATFORM_WINDOWS, getenv_mock_windows_defaults);
  config_builder_build(config_builder, &config);
  TEST(strcmp(config.config_dir, "C:\\Users\\user\\AppData\\Roaming\\malachi") == 0);
  TEST(strcmp(config.data_dir, "C:\\Users\\user\\AppData\\Local\\malachi") == 0);
  config_finish(&config);
  printf("%s:%s: Passed!\n", __FILE__, __func__);
}

char *getenv_mock_macos_defaults(const char *name) {
  if (strcmp(name, "HOME") == 0) {
    return "/Users/user";
  }
  return NULL;
}

void test_config_builder_with_macos_defaults(void) {
  printf("%s:%s: Starting...\n", __FILE__, __func__);
  struct config config = {};
  struct config_builder *config_builder = config_builder_create();
  config_builder_with_defaults(config_builder, PLATFORM_MACOS, getenv_mock_macos_defaults);
  config_builder_build(config_builder, &config);
  TEST(strcmp(config.config_dir, "/Users/user/Library/Application Support/malachi") == 0);
  TEST(strcmp(config.data_dir, "/Users/user/Library/Application Support/malachi") == 0);
  config_finish(&config);
  printf("%s:%s: Passed!\n", __FILE__, __func__);
}

char *getenv_mock_unix_defaults(const char *name) {
  if (strcmp(name, "HOME") == 0) {
    return "/home/user";
  }
  return NULL;
}

void test_config_builder_with_linux_defaults(void) {
  printf("%s:%s: Starting...\n", __FILE__, __func__);
  struct config config = {};
  struct config_builder *config_builder = config_builder_create();
  config_builder_with_defaults(config_builder, PLATFORM_LINUX, getenv_mock_unix_defaults);
  config_builder_build(config_builder, &config);
  TEST(strcmp(config.config_dir, "/home/user/.config/malachi") == 0);
  TEST(strcmp(config.data_dir, "/home/user/.local/share/malachi") == 0);
  config_finish(&config);
  printf("%s:%s: Passed!\n", __FILE__, __func__);
}

void test_config_builder_with_unknown_defaults(void) {
  printf("%s:%s: Starting...\n", __FILE__, __func__);
  struct config config = {};
  struct config_builder *config_builder = config_builder_create();
  config_builder_with_defaults(config_builder, PLATFORM_UNKNOWN, getenv_mock_unix_defaults);
  config_builder_build(config_builder, &config);
  TEST(strcmp(config.config_dir, "/home/user/.config/malachi") == 0);
  TEST(strcmp(config.data_dir, "/home/user/.local/share/malachi") == 0);
  config_finish(&config);
  printf("%s:%s: Passed!\n", __FILE__, __func__);
}

int main(void) {
  test_platform_to_string();
  test_config_builder_with_windows_defaults();
  test_config_builder_with_linux_defaults();
  return EXIT_SUCCESS;
}
