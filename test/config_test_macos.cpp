#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "test.h"

#include "config.h"

namespace {

char *getenv_defaults(const char *name) {
  if (strcmp(name, "HOME") == 0) {
    return (char *)"/Users/user";
  }
  return NULL;
}

void test_config_builder_with_defaults() {
  BEGIN_TEST();
  struct config_builder config_builder = {};
  int rc = config_builder_init(&config_builder, getenv_defaults);
  TEST(rc == 0);
  config_builder_with_defaults(&config_builder);
  struct config config = {};
  struct error error = {};
  rc = config_builder_build(&config_builder, &config, &error);
  TEST(rc == 0);
  TEST(error.rc == rc);
  TEST(error.msg == NULL);
  TEST(strcmp(config.config_dir, "/Users/user/Library/Application Support/malachi") == 0);
  TEST(strcmp(config.data_dir, "/Users/user/Library/Application Support/malachi") == 0);
  config_finish(&config);
  END_TEST();
}

} // namespace

int main() {
  test_config_builder_with_defaults();
  return EXIT_SUCCESS;
}