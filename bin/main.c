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

    struct config_builder *config_builder = config_builder_create();
    config_builder_with_defaults(config_builder, getenv);
    struct config config = {0};
    const int rc = config_builder_build(config_builder, &config);
    if (rc != 0) {
        printf("Failed to build config: %s\n", config_builder_error_to_string(rc));
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
