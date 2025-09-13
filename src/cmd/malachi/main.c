#include "project.h"

#include <assert.h>
#include <fcntl.h>
#include <limits.h>
#include <poll.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
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

int debug = 0;

char const *const appname = "malachi";

static sig_atomic_t volatile running = 1;
static sig_atomic_t volatile sigrecvd = 0;

static char const *const commitstr =
	strlen(MALACHI_COMMIT_SHORT_HASH) > 0
		? "-" MALACHI_COMMIT_SHORT_HASH
		: "";

static void
usage(char *argv[])
{
	eprintf("Usage: %s [-v] [-d] [-c] [-t [name]]\n", argv[0]);
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

static void
sighandler(int sig)
{
	sigrecvd = sig;
	running = 0;
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
			c = getopt(argc, argv, "vdct::");
			if(c == -1)
				break;

			switch(c) {
			case 'v':
				opts.version = 1;
				break;
			case 'd':
				debug = 1;
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
		__label__ cleanup;

		extern int optind;

		int rc = -1;
		int ret = EXIT_FAILURE;
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
			logerror("Failed to initialize config: %s", error.msg);
			return EXIT_FAILURE;
		}

		if(opts.config) {
			configprint(&config);
			ret = EXIT_SUCCESS;
			goto cleanup;
		}

		struct sigaction sa = {
			.sa_handler = sighandler,
			.sa_flags = 0,
		};
		sigemptyset(&sa.sa_mask);

		if(sigaction(SIGINT, &sa, NULL) == -1) {
			logerror("Failed to set SIGINT handler");
			goto cleanup;
		}

		if(sigaction(SIGTERM, &sa, NULL) == -1) {
			logerror("Failed to set SIGTERM handler");
			goto cleanup;
		}

		loginfo("Starting daemon");
		logdebug("Debug logging enabled");

		while(running) {
			logdebug("Event loop iteration");
			sleep(1);
		}

		printf("\n");
		switch(sigrecvd) {
		case SIGINT:
			loginfo("Received SIGINT, shutting down");
			break;
		case SIGTERM:
			loginfo("Received SIGTERM, shutting down");
			break;
		default:
			loginfo("Shutting down");
			break;
		}

		ret = EXIT_SUCCESS;

	cleanup:
		configfree(&config);
		return ret;
	}
}
