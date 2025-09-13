#include <string.h>

#include "malachi.h"

#define MAXFILTERS 16

static Filter const *filters[MAXFILTERS];
static int nfilters = 0;

void
filteradd(Filter const *ops)
{
	if(nfilters < MAXFILTERS - 1) {
		filters[nfilters++] = ops;
		filters[nfilters] = NULL;
	}
}

Filter const *
filterget(char const *ext)
{
	for(int i = 0; i < nfilters; ++i) {
		Filter const *filter = filters[i];
		for(int j = 0; filter->exts[j]; ++j) {
			if(strcmp(ext, filter->exts[j]) == 0)
				return filter;
		}
	}
	return NULL;
}

Filter const **
filterall(void)
{
	return filters;
}
