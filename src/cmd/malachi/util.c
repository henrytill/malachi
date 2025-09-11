#include <stdarg.h>
#include <stdio.h>

#include "dat.h"
#include "fns.h"

int
eprintf(char const *fmt, ...)
{
	va_list args;
	int ret;

	va_start(args, fmt);
	ret = vfprintf(stderr, fmt, args);
	va_end(args);

	return ret;
}
