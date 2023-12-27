#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "test.h"

#include "config.h"

void test_platform_to_string(void) {
  BEGIN_TEST();
  TEST(strcmp(platform_to_string(), "macOS") == 0);
  END_TEST();
}

char *getenv_mock_defaults(const char *name) {
  if (strcmp(name, "HOME") == 0) {
    return "/Users/user";
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
  TEST(strcmp(config.config_dir, "/Users/user/Library/Application Support/malachi") == 0);
  TEST(strcmp(config.data_dir, "/Users/user/Library/Application Support/malachi") == 0);
  config_finish(&config);
  END_TEST();
}

int main(void) {
  test_platform_to_string();
  test_config_builder_with_defaults();
  return EXIT_SUCCESS;
}