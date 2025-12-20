#include "project.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sqlite3.h>

#include <yyjson.h>

#include "malachi.h"

char const *const appname = "malachi";

int debug = 0;

static char const *const commitstr = sizeof(MALACHI_COMMIT_SHORT_HASH) - 1 > 0
    ? "-" MALACHI_COMMIT_SHORT_HASH
    : "";

static sig_atomic_t volatile loopstat = 1;
static sig_atomic_t volatile sigrecvd = 0;

struct Opts
{
    int version;
    int config;
    int test;
    char const *testname;
};

static void usage(char *argv[])
{
    eprintf("Usage: %s [-v] [-d] [-c] [-t [name]]\n", argv[0]);
}

static void yyjsonversionprint(void)
{
    printf("yyjson=%s\n", YYJSON_VERSION_STRING);
}

static int versionprint(void)
{
    printf("malachi=%d.%d.%d%s\n", MALACHI_VERSION_MAJOR, MALACHI_VERSION_MINOR, MALACHI_VERSION_PATCH, commitstr);

    printf("sqlite=%s\n", sqlite3_libversion());

    yyjsonversionprint();

    {
        Filter const **filters = filterall();
        for (int i = 0; filters[i]; ++i)
            printf("%s=%s\n", filters[i]->name, filters[i]->version());
    }

    return 0;
}

static void configprint(Config const *config)
{
    printf("platform=%s\n", platformstr());
    printf("configdir=%s\n", config->configdir);
    printf("datadir=%s\n", config->datadir);
    printf("cachedir=%s\n", config->cachedir);
    printf("runtimedir=%s\n", config->runtimedir);
}

static void sighandler(int sig)
{
    sigrecvd = sig;
    loopstat = 0;
}

static int handlecommand(struct Command const *cmd)
{
    switch (cmd->op)
    {
    case Opadd:
        loginfo("Add repository: %s", cmd->pathop.path);
        return 0;
    case Opremove:
        loginfo("Remove repository: %s", cmd->pathop.path);
        return 0;
    case Opquery:
        loginfo(
            "Query: %s (id=%s, filter=%s)",
            cmd->queryop.terms,
            cmd->queryop.queryid,
            cmd->queryop.repofilter);
        return 0;
    case Opshutdown:
        loginfo("Shutdown requested");
        return 1;
    default:
        logerror("Unknown operation");
        return 0;
    }
}

static void readcommands(int pipefd, Parser *parser, int *generation)
{
    ssize_t nread = parserinput(parser, pipefd);
    if (nread == -Enospace)
    {
        logerror("Parser buffer full, processing pending commands");
    }
    else if (nread == 0)
    {
        /* EOF - no more data available */
        return;
    }
    else if (nread < 0)
    {
        logerror("Read error: %s", strerror(errno));
        return;
    }

    struct Command cmd;
    int result;

    for (;;)
    {
        result = parsecommand(parser, &cmd, generation);
        if (result <= 0)
            break;

        result = handlecommand(&cmd);
        if (result < 0)
            break;
    }

    if (result == 0)
    {
        /* Incomplete command in buffer - normal, wait for more data */
    }
    else if (result < 0)
    {
        logerror("Malformed record skipped, continuing");
    }
}

static int runloop(char const *pipepath)
{
    int ret = -1;
    int pipefd = -1;

    int generation = 0;

    Parser *parser = parsercreate((size_t)MAXRECORDSIZE * 2);
    if (!parser)
    {
        logerror("Failed to create parser");
        return -1;
    }

    struct pollfd pfd = {
        .events = POLLIN,
    };

    goto init;

    while (loopstat)
    {
        pfd.revents = 0;
        int rc = poll(&pfd, 1, 1000);

        if (rc == -1)
        {
            if (errno == EINTR)
                continue;
            logerror("poll failed: %s", strerror(errno));
            goto closepipefd;
        }

        if (rc == 0)
        {
            continue;
        }

        if (pfd.revents & POLLERR)
        {
            logerror("Pipe error occurred");
            goto closepipefd;
        }

        if (pfd.revents & POLLIN)
        {
            readcommands(pipefd, parser, &generation);
        }

        if (pfd.revents & POLLHUP)
        {
            logdebug("Client disconnected, reopening pipe");
            close(pipefd);
            goto init;
        }

        continue;

    init:
        pipefd = open(pipepath, O_RDONLY | O_NONBLOCK);
        if (pipefd == -1)
        {
            if (errno == EINTR)
            {
                logdebug("Signal received during pipe open, exiting");
                ret = 0;
                goto destroyparser;
            }
            logerror("Failed to open command pipe: %s", strerror(errno));
            goto destroyparser;
        }
        parserreset(parser);
        pfd.fd = pipefd;
    }

    ret = 0;

closepipefd:
    close(pipefd);
destroyparser:
    parserdestroy(parser);
    return ret;
}

static int run(Config *config)
{
    int ret = -1;
    Error error = { 0 };

    struct sigaction sa = {
        .sa_handler = sighandler,
        .sa_flags = 0,
    };
    sigemptyset(&sa.sa_mask);

    int rc = sigaction(SIGINT, &sa, NULL);
    if (rc == -1)
    {
        logerror("Failed to set SIGINT handler");
        return -1;
    }

    rc = sigaction(SIGTERM, &sa, NULL);
    if (rc == -1)
    {
        logerror("Failed to set SIGTERM handler");
        return -1;
    }

    Database *database = dbcreate(config, &error);
    if (database == NULL)
    {
        logerror("Failed to initialize database: %s", error.msg);
        return -1;
    }

    rc = mkdirp(config->runtimedir, 0700);
    if (rc == -1)
    {
        logerror("Failed to create runtime directory: %s", strerror(errno));
        goto destroydatabase;
    }

    char *pipepath = joinpath2(config->runtimedir, "command");
    if (pipepath == NULL)
    {
        logerror("Failed to allocate pipe path");
        goto destroydatabase;
    }

    rc = mkfifo(pipepath, 0622);
    if (rc == -1)
    {
        logerror("Failed to mkfifo: %s", strerror(errno));
        goto freepipepath;
    }

    loginfo("Starting daemon");
    loginfo("Command pipe: %s", pipepath);
    logdebug("Debug logging enabled");

    rc = runloop(pipepath);
    if (rc != 0)
        goto unlinkpipepath;

    switch (sigrecvd)
    {
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

unlinkpipepath:
    unlink(pipepath);
freepipepath:
    free(pipepath);
destroydatabase:
    dbdestroy(database);
    return ret;
}

int main(int argc, char *argv[])
{
    int ret = EXIT_FAILURE;
    int rc = -1;
    struct Opts opts = { 0 };
    Config config = { 0 };

    {
        int c = 0;

        for (;;)
        {
            c = getopt(argc, argv, "vdct::");
            if (c == -1)
                break;

            switch (c)
            {
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
        Error error = { 0 };

        if (opts.test)
        {
            int tr;
            if (opts.testname)
                tr = testone(opts.testname);
            else
                tr = testall();

            return tr ? EXIT_FAILURE : EXIT_SUCCESS;
        }

        if (opts.version)
        {
            rc = versionprint();
            return rc ? EXIT_FAILURE : EXIT_SUCCESS;
        }

        rc = configinit(getenv, &config, &error);
        if (rc != 0)
        {
            logerror("Failed to initialize config: %s", error.msg);
            return EXIT_FAILURE;
        }

        if (opts.config)
        {
            configprint(&config);
            ret = EXIT_SUCCESS;
            goto freeconfig;
        }
    }

    rc = run(&config);
    ret = (rc == 0) ? EXIT_SUCCESS : EXIT_FAILURE;

freeconfig:
    configfree(&config);
    return ret;
}
