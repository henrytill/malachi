#include <getopt.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>

#include <git2/common.h>
#include <mupdf/fitz.h>
#include <sqlite3.h>

#include "config.h"
#include "error.h"
#include "platform.h"

#define eprintf(...) (void)fprintf(stderr, __VA_ARGS__)

struct malachi_opts {
  int version;
  int config;
};

static void usage(char *argv[]) {
  eprintf("Usage: %s [--version] [--config] <query>\n", argv[0]);
}

static int print_versions(void) {
  {
    int major = 0;
    int minor = 0;
    int rev = 0;
    const int rc = git_libgit2_version(&major, &minor, &rev);
    if (rc != 0) {
      eprintf("Failed to get libgit2 version\n");
      return -1;
    }
    printf("libgit2: %d.%d.%d\n", major, minor, rev);
  }
  printf("mupdf: %s\n", FZ_VERSION);
  printf("sqlite: %s\n", sqlite3_libversion());
  return 0;
}

static int configure(struct config *config) {
  struct error error = {0};
  struct config_builder builder = {0};
  int rc = config_builder_init(&builder, getenv);
  if (rc != 0) {
    eprintf("Failed to create config_builder\n");
    return -1;
  }
  config_builder_with_defaults(&builder);
  rc = config_builder_build(&builder, config, &error);
  if (rc != 0) {
    eprintf("Failed to build config: %s\n", error.msg);
    return -1;
  }
  return 0;
}

static void print_config(const struct config *config) {
  printf("platform: %s\n", platform_to_string());
  printf("config_dir: %s\n", config->config_dir);
  printf("data_dir: %s\n", config->data_dir);
}

int main(int argc, char *argv[]) {
  struct malachi_opts opts = {0};

  if (argc == 1) {
    usage(argv);
    return EXIT_FAILURE;
  }

  {
    int c = 0;
    int option_index = 0;

    struct option long_options[] = {
      {"version", no_argument, &opts.version, 1},
      {"config", no_argument, &opts.config, 1},
      {0, 0, 0, 0},
    };

    for (;;) {
      option_index = 0;

      c = getopt_long(argc, argv, "vc", long_options, &option_index);
      if (c == -1) {
        break;
      }

      switch (c) {
      case 'v':
        opts.version = 1;
        break;
      case 'c':
        opts.config = 1;
        break;
      case '?':
        usage(argv);
        return EXIT_FAILURE;
      default:
        break;
      }
    }
  }

  {
    extern int optind;

    int rc = -1;
    struct config config = {0};

    if (opts.version) {
      rc = print_versions();
      return rc ? EXIT_FAILURE : EXIT_SUCCESS;
    }

    rc = configure(&config);
    if (rc != 0) {
      return EXIT_FAILURE;
    }

    if (opts.config) {
      print_config(&config);
      config_finish(&config);
      return EXIT_SUCCESS;
    }

    if (optind < argc) {
      printf("non-option argv elements: ");
      while (optind < argc) {
        printf("%s ", argv[optind++]);
      }
      printf("\n");
    }

    {
      char *cwd = getcwd(NULL, 0);
      printf("cwd: %s\n", cwd);
      free(cwd);
    }

    config_finish(&config);
  }

  return EXIT_SUCCESS;
}
