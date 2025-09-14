#include "project.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <git2/common.h>
#include <sqlite3.h>

#include "malachi.h"

enum {
	LINEBUFLEN = 1024,
	MAXHASHLEN = 65,
};

char const *const appname = "malachi";

int debug = 0;

static char const *const commitstr =
	strlen(MALACHI_COMMIT_SHORT_HASH) > 0
		? "-" MALACHI_COMMIT_SHORT_HASH
		: "";

static sig_atomic_t volatile loopstat = 1;
static sig_atomic_t volatile sigrecvd = 0;

struct Opts {
	int version;
	int config;
	int test;
	char const *testname;
};

enum {
	Opunknown = 0,
	Opadded,
	Opchanged,
	Opremoved,
	Opshutdown,
};

struct Command {
	int op;
	char repo[PATH_MAX];
	char path[PATH_MAX];
	char hash[MAXHASHLEN];
};

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
	printf("runtimedir=%s\n", config->runtimedir);
}

static void
sighandler(int sig)
{
	sigrecvd = sig;
	loopstat = 0;
}

static int
parseop(char const *opstr)
{
	if(strcmp(opstr, "added") == 0)
		return Opadded;

	if(strcmp(opstr, "changed") == 0)
		return Opchanged;

	if(strcmp(opstr, "removed") == 0)
		return Opremoved;

	if(strcmp(opstr, "shutdown") == 0)
		return Opshutdown;

	return Opunknown;
}

static int
parseline(char const *line, struct Command *cmd)
{
	memset(cmd, 0, sizeof(*cmd));

	char *dupline = strdup(line);
	if(!dupline)
		return -1;

	char *p;
	char *token = strtok_r(dupline, " \t\n", &p);

	while(token) {
		char *eq = strchr(token, '=');
		if(!eq) {
			token = strtok_r(NULL, " \t\n", &p);
			continue;
		}

		*eq = '\0';
		char *key = token;
		char *value = eq + 1;

		if(value[0] == '"') {
			value++;
			char *endq = strrchr(value, '"');
			if(endq)
				*endq = '\0';
		}

		if(strcmp(key, "op") == 0) {
			cmd->op = parseop(value);
		} else if(strcmp(key, "repo") == 0) {
			strncpy(cmd->repo, value, sizeof(cmd->repo) - 1);
		} else if(strcmp(key, "path") == 0) {
			strncpy(cmd->path, value, sizeof(cmd->path) - 1);
		} else if(strcmp(key, "hash") == 0) {
			strncpy(cmd->hash, value, sizeof(cmd->hash) - 1);
		}

		token = strtok_r(NULL, " \t\n", &p);
	}

	free(dupline);
	return 0;
}

static int
handlecommand(struct Command const *cmd)
{
	switch(cmd->op) {
	case Opadded:
		loginfo("Adding file: %s in repository %s (hash: %s)",
			cmd->path,
			cmd->repo,
			cmd->hash);
		return 0;
	case Opchanged:
		loginfo("Updating file: %s in repository %s (hash: %s)",
			cmd->path,
			cmd->repo,
			cmd->hash);
		return 0;
	case Opremoved:
		loginfo("Removing file: %s from repository %s (hash: %s)",
			cmd->path,
			cmd->repo,
			cmd->hash);
		return 0;
	case Opshutdown:
		loginfo("Shutdown requested");
		return 1;
	default:
		logerror("Unknown operation");
		return 0;
	}
}

static void
handleline(char const *line)
{
	struct Command cmd;
	int parsed = parseline(line, &cmd);
	if(parsed != 0)
		logerror("Failed to parse line: %s", line);

	int shutdown = handlecommand(&cmd);
	if(shutdown == 1)
		loopstat = 0;
}

static void
readlines(int pipefd, char *buf, size_t bufsize, size_t *bufused, void (*f)(char const *line))
{
	size_t used = *bufused;

	ssize_t nreads = read(pipefd, buf + used, bufsize - used - 1);
	if(nreads == 0) {
		logdebug("Pipe EOF");
		return;
	}
	if(nreads < 0 && (errno != EAGAIN && errno != EWOULDBLOCK)) {
		logerror("Read error: %s", strerror(errno));
		return;
	}

	used += nreads;
	buf[used] = '\0';

	char *p = NULL;
	char *nl = NULL;
	for(p = buf; (nl = memchr(p, '\n', used - (p - buf))); p = nl + 1) {
		*nl = '\0';
		if(strlen(p) > 0)
			f(p);
	}

	size_t remaining = used - (p - buf);
	memmove(buf, p, remaining);
	*bufused = remaining;
}

static int
runloop(char const *pipepath)
{
	int ret = -1;
	int pipefd = -1;
	static char buf[4096];
	static size_t bufused = 0;

	struct pollfd pfd = {
		.events = POLLIN,
	};

	goto init;

	while(loopstat) {
		pfd.revents = 0;
		int rc = poll(&pfd, 1, 1000);

		if(rc == -1) {
			if(errno == EINTR)
				continue;
			logerror("poll failed: %s", strerror(errno));
			goto out_close_pipefd;
		}

		if(rc == 0) {
			logdebug(".");
			continue;
		}

		if(pfd.revents & POLLERR) {
			logerror("Pipe error occurred");
			goto out_close_pipefd;
		}

		if(pfd.revents & POLLIN) {
			readlines(pipefd, buf, sizeof(buf), &bufused, handleline);
		}

		if(pfd.revents & POLLHUP) {
			logdebug("Client disconnected, reopening pipe");
			close(pipefd);
			goto init;
		}

		continue;

	init:
		pipefd = open(pipepath, O_RDONLY | O_NONBLOCK);
		if(pipefd == -1) {
			if(errno == EINTR) {
				logdebug("Signal received during pipe open, exiting");
				return 0;
			}
			logerror("Failed to open command pipe: %s", strerror(errno));
			return -1;
		}

		bufused = 0; // Reset buffer on pipe reopen
		pfd.fd = pipefd;
	}

	ret = 0;

out_close_pipefd:
	close(pipefd);
	return ret;
}

static int
run(Config *config)
{
	int ret = -1;
	Error error = {0};

	struct sigaction sa = {
		.sa_handler = sighandler,
		.sa_flags = 0,
	};
	sigemptyset(&sa.sa_mask);

	int rc = sigaction(SIGINT, &sa, NULL);
	if(rc == -1) {
		logerror("Failed to set SIGINT handler");
		return -1;
	}

	rc = sigaction(SIGTERM, &sa, NULL);
	if(rc == -1) {
		logerror("Failed to set SIGTERM handler");
		return -1;
	}

	Database *database = dbcreate(config, &error);
	if(database == NULL) {
		logerror("Failed to initialize database: %s", error.msg);
		return -1;
	}

	rc = mkdirp(config->runtimedir, 0700);
	if(rc == -1) {
		logerror("Failed to create runtime directory: %s", strerror(errno));
		goto out_dbdestroy_database;
	}

	char *pipepath = joinpath2(config->runtimedir, "command");
	if(pipepath == NULL) {
		logerror("Failed to allocate pipe path");
		goto out_dbdestroy_database;
	}

	rc = mkfifo(pipepath, 0622);
	if(rc == -1) {
		logerror("Failed to mkfifo: %s", strerror(errno));
		goto out_free_pipepath;
	}

	loginfo("Starting daemon");
	loginfo("Command pipe: %s", pipepath);
	logdebug("Debug logging enabled");

	rc = runloop(pipepath);
	if(rc != 0)
		goto out_unlink_pipepath;

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

	ret = 0;

out_unlink_pipepath:
	unlink(pipepath);
out_free_pipepath:
	free(pipepath);
out_dbdestroy_database:
	dbdestroy(database);
	return ret;
}

int
main(int argc, char *argv[])
{
	int ret = EXIT_FAILURE;
	int rc = -1;
	struct Opts opts = {0};
	Config config = {0};

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
			goto out_configfree;
		}
	}

	rc = run(&config);
	ret = (rc == 0) ? EXIT_SUCCESS : EXIT_FAILURE;

out_configfree:
	configfree(&config);
	return ret;
}
