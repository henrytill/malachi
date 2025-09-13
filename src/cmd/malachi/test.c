#include <stdio.h>
#include <string.h>

#include "malachi.h"

#define MAXTESTS 16

static Test const *tests[MAXTESTS];
static int ntests = 0;

void
testadd(Test const *ops)
{
	if(ntests < MAXTESTS - 1) {
		tests[ntests++] = ops;
		tests[ntests] = NULL;
	}
}

int
testall(void)
{
	int failed = 0;

	printf("Running %d tests...\n", ntests);

	for(int i = 0; i < ntests; ++i) {
		Test const *test = tests[i];
		printf("Running test: %s... ", test->name);
		(void)fflush(stdout);

		int result = test->run();
		if(result == 0) {
			printf("PASS\n");
			continue;
		}

		printf("FAIL\n");
		failed++;
	}

	printf("\nResults: %d passed, %d failed\n", ntests - failed, failed);
	return failed;
}

int
testone(char const *name)
{
	for(int i = 0; i < ntests; ++i) {
		Test const *test = tests[i];
		if(strcmp(test->name, name) == 0) {
			printf("Running test: %s... ", test->name);
			(void)fflush(stdout);

			int result = test->run();
			if(result == 0) {
				printf("PASS\n");
				return 0;
			}

			printf("FAIL\n");
			return 1;
		}
	}

	printf("Test '%s' not found\n", name);
	return 1;
}
