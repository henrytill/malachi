#include <stdarg.h>
#include <stdio.h>

#include "dat.h"
#include "fns.h"

int
eprintf(char *fmt, ...)
{
	va_list arg;
	int n;

	va_start(arg, fmt);
	n = vfprintf(stderr, fmt, arg);
	va_end(arg);

	return n;
}
