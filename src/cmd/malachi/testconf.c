#include "malachi.h"

int testconfdef(void);
int testplatspec(void);

static char *
getenvmt(char const *name)
{
	(void)name;
	return NULL;
}

static int
testconfmiss(void)
{
	Config config = {0};
	Error error = {0};
	int rc = configinit(getenvmt, &config, &error);
	if(rc != -Emissingdir) {
		eprintf("expected rc=%d, got rc=%d\n", -Emissingdir, rc);
		return -1;
	}
	if(error.rc != -Emissingdir) {
		eprintf("expected error.rc=%d, got error.rc=%d\n", -Emissingdir, error.rc);
		return -1;
	}
	if(strcmp(error.msg, "configdir is NULL") != 0) {
		eprintf("expected error.msg='configdir is NULL', got '%s'\n", error.msg);
		return -1;
	}
	configfree(&config);
	return 0;
}

static int
run(void)
{
	int failures = 0;

	if(testconfdef() != 0)
		failures++;
	if(testplatspec() != 0)
		failures++;
	if(testconfmiss() != 0)
		failures++;

	return failures;
}

static Test const test = {
	.name = "config",
	.run = run,
};

__attribute__((constructor)) static void
init(void)
{
	testadd(&test);
}
