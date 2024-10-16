#include "path.h"

#include <stdio.h>
#include <stdlib.h>

#include "platform.h"

#define SEPARATOR '/'

char *joinpath2(const char *a, const char *b)
{
    size_t len = (size_t)snprintf(NULL, 0, "%s%c%s", a, SEPARATOR, b);
    char *ret = calloc(++len, sizeof(*ret)); // incr for terminator
    if (ret == NULL) {
        return NULL;
    }
    (void)snprintf(ret, len, "%s%c%s", a, SEPARATOR, b);
    return ret;
}

char *joinpath3(const char *a, const char *b, const char *c)
{
    size_t len = (size_t)snprintf(NULL, 0, "%s%c%s%c%s", a, SEPARATOR, b, SEPARATOR, c);
    char *ret = calloc(++len, sizeof(*ret)); // incr for terminator
    if (ret == NULL) {
        return NULL;
    }
    (void)snprintf(ret, len, "%s%c%s%c%s", a, SEPARATOR, b, SEPARATOR, c);
    return ret;
}

char *joinpath4(const char *a, const char *b, const char *c, const char *d)
{
    size_t len = (size_t)snprintf(NULL, 0, "%s%c%s%c%s%c%s", a, SEPARATOR, b, SEPARATOR, c, SEPARATOR, d);
    char *ret = calloc(++len, sizeof(*ret)); // incr for terminator
    if (ret == NULL) {
        return NULL;
    }
    (void)snprintf(ret, len, "%s%c%s%c%s%c%s", a, SEPARATOR, b, SEPARATOR, c, SEPARATOR, d);
    return ret;
}
