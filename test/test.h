#pragma once

#include <stdio.h> // IWYU pragma: keep
#include <stdlib.h>

#define TAG(s)       printf("%s:%d:%s: %s\n", __FILE__, __LINE__, __func__, (s))
#define BEGIN_TEST() TAG("Starting...")
#define END_TEST()   TAG("Passed!")

#define TEST(e)                                                               \
	if (!(e)) {                                                           \
		(void)fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #e); \
		exit(EXIT_FAILURE);                                           \
	}
