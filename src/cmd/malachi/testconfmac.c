#include <stdlib.h>

#include "malachi.h"

static char *getenvdef(char const *name)
{
    if (strcmp(name, "HOME") == 0)
        return "/Users/test";

    return NULL;
}

int testconfdef(void)
{
    Config config = { 0 };
    Error error = { 0 };
    int rc = configinit(getenvdef, &config, &error);
    if (rc != 0)
    {
        eprintf("configinit failed with rc=%d\n", rc);
        return -1;
    }
    if (error.rc != rc)
    {
        eprintf("error.rc mismatch\n");
        return -1;
    }
    if (error.msg != NULL)
    {
        eprintf("error.msg should be NULL\n");
        return -1;
    }
    if (strcmp(config.configdir, "/Users/test/Library/Application Support/malachi") != 0)
    {
        eprintf("configdir mismatch: %s\n", config.configdir);
        return -1;
    }
    if (strcmp(config.datadir, "/Users/test/Library/Application Support/malachi") != 0)
    {
        eprintf("datadir mismatch: %s\n", config.datadir);
        return -1;
    }
    configfree(&config);
    return 0;
}

int testplatspec(void)
{
    return 0;
}
