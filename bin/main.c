#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "platform.h"

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <filename>\n", argv[0]);
    return EXIT_FAILURE;
  }

  const enum platform p = platform_get();

  printf("Platform: %s\n", platform_to_string(p));

  struct config config = {};
  struct config_builder *config_builder = config_builder_create();
  config_builder_with_defaults(config_builder, p, getenv);
  config_builder_build(config_builder, &config);

  printf("Config dir: %s\n", config.config_dir);
  printf("Data dir: %s\n", config.data_dir);

  config_finish(&config);
  return EXIT_SUCCESS;
}
