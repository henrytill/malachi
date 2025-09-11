#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dat.h"
#include "fns.h"

#define MAX_TESTS 16

static struct test_ops const *all_tests[MAX_TESTS];
static int test_count = 0;

void
test_register(struct test_ops const *ops)
{
	if (test_count < MAX_TESTS - 1) {
		all_tests[test_count++] = ops;
		all_tests[test_count] = NULL;
	}
}

int
test_run_all(void)
{
	int failed = 0;

	printf("Running %d tests...\n", test_count);

	for (int i = 0; i < test_count; ++i) {
		struct test_ops const *test = all_tests[i];
		printf("Running test: %s... ", test->name);
		(void)fflush(stdout);

		int result = test->run();
		if (result == 0) {
			printf("PASS\n");
			continue;
		}

		printf("FAIL\n");
		failed++;
	}

	printf("\nResults: %d passed, %d failed\n", test_count - failed, failed);
	return failed;
}

int
test_run_by_name(char const *name)
{
	for (int i = 0; i < test_count; ++i) {
		struct test_ops const *test = all_tests[i];
		if (strcmp(test->name, name) == 0) {
			printf("Running test: %s... ", test->name);
			(void)fflush(stdout);

			int result = test->run();
			if (result == 0) {
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
