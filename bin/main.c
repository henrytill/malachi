#include <stdio.h>
#include <stdlib.h>

#include <mupdf/fitz.h>
#include <sqlite3.h>

#include "config.h"
#include "platform.h"

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <filename>\n", argv[0]);
    return EXIT_FAILURE;
  }

  const enum platform p = platform_get();
  struct config_builder *config_builder = config_builder_create();
  config_builder_with_defaults(config_builder, p, getenv);
  struct config_builder_result result = config_builder_build(config_builder);
  if (result.tag == CONFIG_BUILDER_RESULT_ERR) {
    printf("Error: %s\n", result.data.err);
    return EXIT_FAILURE;
  }

  struct config config = result.data.ok;

  printf("platform: %s\n", platform_to_string(p));
  printf("config_dir: %s\n", config.config_dir);
  printf("data_dir: %s\n", config.data_dir);
  printf("sqlite: %s\n", sqlite3_libversion());
  printf("mupdf: %s\n", FZ_VERSION);

  config_finish(&config);
  return EXIT_SUCCESS;
}
