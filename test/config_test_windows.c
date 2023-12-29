#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "test.h"

#include "config.h"

char *getenv_mock_defaults(const char *name) {
  if (strcmp(name, "APPDATA") == 0) {
    return "C:\\Users\\user\\AppData\\Roaming";
  }
  if (strcmp(name, "LOCALAPPDATA") == 0) {
    return "C:\\Users\\user\\AppData\\Local";
  }
  return NULL;
}

void test_config_builder_with_defaults(void) {
  BEGIN_TEST();
  struct config_builder *config_builder = config_builder_create();
  config_builder_with_defaults(config_builder, getenv_mock_defaults);
  struct config config = {0};
  const int rc = config_builder_build(config_builder, &config);
  TEST(rc == 0);
  TEST(strcmp(config.config_dir, "C:\\Users\\user\\AppData\\Roaming\\malachi") == 0);
  TEST(strcmp(config.data_dir, "C:\\Users\\user\\AppData\\Local\\malachi") == 0);
  config_finish(&config);
  END_TEST();
}

char *getenv_mock_windows_no_appdata(const char *name) {
  if (strcmp(name, "LOCALAPPDATA") == 0) {
    return "C:\\Users\\user\\AppData\\Local";
  }
  return NULL;
}

void test_config_builder_no_config_dir(void) {
  BEGIN_TEST();
  struct config_builder *config_builder = config_builder_create();
  config_builder_with_defaults(config_builder, getenv_mock_windows_no_appdata);
  struct config config = {0};
  const int rc = config_builder_build(config_builder, &config);
  TEST(rc == CONFIG_BUILDER_ERROR_MISSING_CONFIG_DIR);
  TEST(strcmp(config_builder_error_to_string(rc), "config_dir is NULL") == 0);
  config_finish(&config);
  END_TEST();
}

char *getenv_mock_windows_no_localappdata(const char *name) {
  if (strcmp(name, "APPDATA") == 0) {
    return "C:\\Users\\user\\AppData\\Roaming";
  }
  return NULL;
}

void test_config_builder_no_data_dir(void) {
  BEGIN_TEST();
  struct config_builder *config_builder = config_builder_create();
  config_builder_with_defaults(config_builder, getenv_mock_windows_no_localappdata);
  struct config config = {0};
  const int rc = config_builder_build(config_builder, &config);
  TEST(rc == CONFIG_BUILDER_ERROR_MISSING_DATA_DIR);
  TEST(strcmp(config_builder_error_to_string(rc), "data_dir is NULL") == 0);
  config_finish(&config);
  END_TEST();
}

int main(void) {
  test_config_builder_with_defaults();
  test_config_builder_no_config_dir();
  test_config_builder_no_data_dir();
  return EXIT_SUCCESS;
}
