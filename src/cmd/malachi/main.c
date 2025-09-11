#include "project.h"

#include <assert.h>
#include <getopt.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <git2/common.h>
#include <sqlite3.h>

#include "dat.h"
#include "fns.h"

struct Opts {
	int version;
	int config;
	int test;
	char const *testname;
};

static void
usage(char *argv[])
{
	eprintf("Usage: %s [--version] [--config] [--test [name]] <query>\n", argv[0]);
}

static void
filterversions(void)
{
	Filter const **filters = filterall();
	for (int i = 0; filters[i]; ++i) {
		printf("%s: %s\n", filters[i]->name, filters[i]->version());
	}
}

static int
versions(void)
{
	printf("malachi: %d.%d.%d", MALACHI_VERSION_MAJOR, MALACHI_VERSION_MINOR, MALACHI_VERSION_PATCH);
	if (strlen(MALACHI_COMMIT_SHORT_HASH) > 0) {
		printf("-%s", MALACHI_COMMIT_SHORT_HASH);
	}
	printf("\n");
	{
		int major = 0;
		int minor = 0;
		int rev = 0;
		int const rc = git_libgit2_version(&major, &minor, &rev);
		if (rc != 0) {
			eprintf("Failed to get libgit2 version\n");
			return -1;
		}
		printf("libgit2: %d.%d.%d\n", major, minor, rev);
	}
	filterversions();
	printf("sqlite: %s\n", sqlite3_libversion());
	return 0;
}

static int
configure(Config *config)
{
	Error error = {0};
	int rc = configinit(getenv, config, &error);
	if (rc != 0) {
		eprintf("Failed to initialize config: %s\n", error.msg);
		return -1;
	}
	return 0;
}

static void
printconfig(Config const *config)
{
	printf("platform: %s\n", platformstr());
	printf("configdir: %s\n", config->configdir);
	printf("datadir: %s\n", config->datadir);
}

int
main(int argc, char *argv[])
{
	struct Opts opts = {0};

	if (argc == 1) {
		usage(argv);
		return EXIT_FAILURE;
	}

	{
		int c = 0;
		int idx = 0;

		struct option longopts[] = {
			{"version", no_argument, NULL, 'v'},
			{"config", no_argument, NULL, 'c'},
			{"test", optional_argument, NULL, 't'},
			{0, 0, 0, 0},
		};

		for (;;) {
			c = getopt_long(argc, argv, "vct::", longopts, &idx);
			if (c == -1)
				break;

			switch (c) {
			case 'v':
				opts.version = 1;
				break;
			case 'c':
				opts.config = 1;
				break;
			case 't':
				opts.test = 1;
				opts.testname = optarg;
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
		Config config = {0};

		if (opts.test) {
			int tr;
			if (opts.testname) {
				tr = testrun(opts.testname);
			} else {
				tr = testall();
			}
			configfree(&config);
			return tr ? EXIT_FAILURE : EXIT_SUCCESS;
		}

		if (opts.version) {
			rc = versions();
			return rc ? EXIT_FAILURE : EXIT_SUCCESS;
		}

		rc = configure(&config);
		if (rc != 0)
			return EXIT_FAILURE;

		if (opts.config) {
			printconfig(&config);
			configfree(&config);
			return EXIT_SUCCESS;
		}

		if (optind < argc) {
			printf("non-option argv elements: ");
			while (optind < argc)
				printf("%s ", argv[optind++]);
			printf("\n");
		}

		{
			// NOLINTNEXTLINE(clang-analyzer-unix.StdCLibraryFunctions)
			char *cwd = getcwd(NULL, 0);
			printf("cwd: %s\n", cwd);
			if (cwd != NULL)
				free(cwd);
		}

		configfree(&config);
	}

	return EXIT_SUCCESS;
}
