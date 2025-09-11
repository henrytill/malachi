#include <stdlib.h>
#include <string.h>

#include "dat.h"
#include "fns.h"

#define MAX_FILTERS 16

static struct filter_ops const *all_filters[MAX_FILTERS];
static int filter_count = 0;

void
filter_register(struct filter_ops const *ops)
{
	if (filter_count < MAX_FILTERS - 1) {
		all_filters[filter_count++] = ops;
		all_filters[filter_count] = NULL;
	}
}

struct filter_ops const *
filter_for_extension(char const *ext)
{
	for (int i = 0; i < filter_count; i++) {
		struct filter_ops const *filter = all_filters[i];
		for (int j = 0; filter->extensions[j]; j++) {
			if (strcmp(ext, filter->extensions[j]) == 0)
				return filter;
		}
	}
	return NULL;
}

struct filter_ops const **
filter_get_all(void)
{
	return all_filters;
}
