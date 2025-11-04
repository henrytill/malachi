#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "malachi.h"

static char const separator = '/';

char *joinpath2(char const *a, char const *b)
{
    int const n = snprintf(NULL, 0, "%s%c%s", a, separator, b);
    size_t len = (assert(n >= 0), (size_t)n);
    char *ret = calloc(++len, sizeof(*ret)); // incr for terminator
    if (ret == NULL)
        return NULL;

    (void)snprintf(ret, len, "%s%c%s", a, separator, b);
    return ret;
}

char *joinpath3(char const *a, char const *b, char const *c)
{
    int const n = snprintf(NULL, 0, "%s%c%s%c%s", a, separator, b, separator, c);
    size_t len = (assert(n >= 0), (size_t)n);
    char *ret = calloc(++len, sizeof(*ret)); // incr for terminator
    if (ret == NULL)
        return NULL;

    (void)snprintf(ret, len, "%s%c%s%c%s", a, separator, b, separator, c);
    return ret;
}

char *joinpath4(char const *a, char const *b, char const *c, char const *d)
{
    int const n = snprintf(NULL, 0, "%s%c%s%c%s%c%s", a, separator, b, separator, c, separator, d);
    size_t len = (assert(n >= 0), (size_t)n);
    char *ret = calloc(++len, sizeof(*ret)); // incr for terminator
    if (ret == NULL)
        return NULL;

    (void)snprintf(ret, len, "%s%c%s%c%s%c%s", a, separator, b, separator, c, separator, d);
    return ret;
}

int mkdirp(char const *path, mode_t mode)
{
    if (path == NULL || *path == '\0') {
        errno = EINVAL;
        return -1;
    }

    size_t pathlen = strlen(path) + 1;
    if (pathlen >= PATH_MAX) {
        errno = ENAMETOOLONG;
        return -1;
    }

    char *pathcopy = malloc(pathlen);
    if (!pathcopy) {
        errno = ENOMEM;
        return -1;
    }

    memcpy(pathcopy, path, pathlen);

    int ret = -1;

    for (char *p = (*pathcopy == '/') ? pathcopy + 1 : pathcopy; *p; ++p) {
        if (*p != '/')
            continue;

        *p = '\0';
        if (mkdir(pathcopy, mode) != 0 && errno != EEXIST)
            goto cleanup;
        *p = '/';
    }

    if (mkdir(pathcopy, mode) != 0 && errno != EEXIST)
        goto cleanup;

    ret = 0;

cleanup:
    free(pathcopy);
    return ret;
}
