#include "project.h"

#include <assert.h>
#include <limits.h>
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

static char const *const commitstr =
	strlen(MALACHI_COMMIT_SHORT_HASH) > 0
		? "-" MALACHI_COMMIT_SHORT_HASH
		: "";

static void
usage(char *argv[])
{
	eprintf("Usage: %s [-v] [-c] [-t [name]] [query]\n", argv[0]);
}

static int
versionprint(void)
{
	printf("malachi=%d.%d.%d%s\n",
		MALACHI_VERSION_MAJOR,
		MALACHI_VERSION_MINOR,
		MALACHI_VERSION_PATCH,
		commitstr);

	{
		int major = 0;
		int minor = 0;
		int rev = 0;
		int const rc = git_libgit2_version(&major, &minor, &rev);
		if(rc != 0) {
			eprintf("Failed to get libgit2 version\n");
			return -1;
		}
		printf("libgit2=%d.%d.%d\n", major, minor, rev);
	}

	printf("sqlite=%s\n", sqlite3_libversion());

	{
		Filter const **filters = filterall();
		for(int i = 0; filters[i]; ++i)
			printf("%s=%s\n",
				filters[i]->name,
				filters[i]->version());
	}

	return 0;
}

static void
configprint(Config const *config)
{
	printf("platform=%s\n", platformstr());
	printf("configdir=%s\n", config->configdir);
	printf("datadir=%s\n", config->datadir);
	printf("cachedir=%s\n", config->cachedir);
}

int
main(int argc, char *argv[])
{
	struct Opts opts = {0};

	if(argc == 1) {
		usage(argv);
		return EXIT_FAILURE;
	}

	{
		int c = 0;

		for(;;) {
			c = getopt(argc, argv, "vct::");
			if(c == -1)
				break;

			switch(c) {
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
		Error error = {0};

		if(opts.test) {
			int tr;
			if(opts.testname)
				tr = testone(opts.testname);
			else
				tr = testall();

			return tr ? EXIT_FAILURE : EXIT_SUCCESS;
		}

		if(opts.version) {
			rc = versionprint();
			return rc ? EXIT_FAILURE : EXIT_SUCCESS;
		}

		rc = configinit(getenv, &config, &error);
		if(rc != 0) {
			eprintf("Failed to initialize config: %s\n", error.msg);
			return EXIT_FAILURE;
		}

		if(opts.config) {
			configprint(&config);
			configfree(&config);
			return EXIT_SUCCESS;
		}

		if(optind < argc) {
			printf("non-option argv elements: ");
			while(optind < argc)
				printf("%s ", argv[optind++]);
			printf("\n");
		}

		{
			char *cwd = realpath(".", NULL);
			printf("cwd: %s\n", cwd);
			if(cwd != NULL)
				free(cwd);
		}

		configfree(&config);
	}

	return EXIT_SUCCESS;
}
