#include <stddef.h>
#include <stdio.h>

#include <mupdf/fitz.h>
#include <sqlite3.h>

#include "config.h"
#include "error.h"
#include "platform.h"

static int configure(struct config *config) {
  struct error error = {0};
  struct config_builder *config_builder = config_builder_create(getenv);
  if (config_builder == NULL) {
    (void)fprintf(stderr, "Failed to create config_builder\n");
    return -1;
  }
  config_builder_with_defaults(config_builder);
  const int rc = config_builder_build(config_builder, config, &error);
  if (rc != 0) {
    (void)fprintf(stderr, "Failed to build config: %s\n", error.msg);
    return -1;
  }
  return 0;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <filename>\n", argv[0]);
    return EXIT_FAILURE;
  }

  struct config config = {0};
  const int rc = configure(&config);
  if (rc != 0) {
    return EXIT_FAILURE;
  }

  printf("platform: %s\n", platform_to_string());
  printf("config_dir: %s\n", config.config_dir);
  printf("data_dir: %s\n", config.data_dir);
  printf("sqlite: %s\n", sqlite3_libversion());
  printf("mupdf: %s\n", FZ_VERSION);

  config_finish(&config);
  return EXIT_SUCCESS;
}
