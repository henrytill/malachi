#include <stdlib.h>

#include "malachi.h"

static char *getenvdef(char const *name)
{
    if (strcmp(name, "HOME") == 0)
        return "/home/user";

    return NULL;
}

int testconfdef(void)
{
    Config config = { 0 };
    Error error = { 0 };
    int rc = configinit(getenvdef, &config, &error);
    if (rc != 0) {
        eprintf("configinit failed with rc=%d\n", rc);
        return -1;
    }
    if (error.rc != rc) {
        eprintf("error.rc mismatch\n");
        return -1;
    }
    if (error.msg != NULL) {
        eprintf("error.msg should be NULL\n");
        return -1;
    }
    if (strcmp(config.configdir, "/home/user/.config/malachi") != 0) {
        eprintf("configdir mismatch: %s\n", config.configdir);
        return -1;
    }
    if (strcmp(config.datadir, "/home/user/.local/share/malachi") != 0) {
        eprintf("datadir mismatch: %s\n", config.datadir);
        return -1;
    }
    configfree(&config);
    return 0;
}

static char *getenvxdg(char const *name)
{
    if (strcmp(name, "XDG_CONFIG_HOME") == 0)
        return "/tmp/config";

    if (strcmp(name, "XDG_DATA_HOME") == 0)
        return "/tmp/data";

    if (strcmp(name, "XDG_CACHE_HOME") == 0)
        return "/tmp/cache";

    return NULL;
}

static int testconfxdg(void)
{
    Config config = { 0 };
    Error error = { 0 };
    int rc = configinit(getenvxdg, &config, &error);
    if (rc != 0) {
        eprintf("configinit failed with rc=%d\n", rc);
        return -1;
    }
    if (error.rc != rc) {
        eprintf("error.rc mismatch\n");
        return -1;
    }
    if (error.msg != NULL) {
        eprintf("error.msg should be NULL\n");
        return -1;
    }
    if (strcmp(config.configdir, "/tmp/config/malachi") != 0) {
        eprintf("configdir mismatch: %s\n", config.configdir);
        return -1;
    }
    if (strcmp(config.datadir, "/tmp/data/malachi") != 0) {
        eprintf("datadir mismatch: %s\n", config.datadir);
        return -1;
    }
    configfree(&config);
    return 0;
}

int testplatspec(void)
{
    return testconfxdg();
}
