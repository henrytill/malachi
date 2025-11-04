#include <stdarg.h>
#include <stdio.h>

#include "malachi.h"

int eprintf(char *fmt, ...)
{
    va_list arg;
    int n;

    va_start(arg, fmt);
    n = vfprintf(stderr, fmt, arg);
    va_end(arg);

    return n;
}

void loginfo(char const *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    printf("[INFO] ");
    vprintf(fmt, args);
    printf("\n");
    va_end(args);
}

void logerror(char const *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    (void)fprintf(stderr, "[ERROR] ");
    (void)vfprintf(stderr, fmt, args);
    (void)fprintf(stderr, "\n");
    va_end(args);
}

void logdebug(char const *fmt, ...)
{
    if (!debug)
        return;
    va_list args;
    va_start(args, fmt);
    printf("[DEBUG] ");
    vprintf(fmt, args);
    printf("\n");
    va_end(args);
}
