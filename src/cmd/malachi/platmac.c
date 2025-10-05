#include <assert.h>
#include <stdlib.h>

#include "malachi.h"

char *platformstr(void) {
  return "macOS";
}

static char *appsupport(Getenvfn getenv, char const *name) {
  assert(name != NULL);

  char const *home = getenv("HOME");
  if (home != NULL)
    return joinpath4(home, "Library", "Application Support", name);

  return NULL;
}

static char *caches(Getenvfn getenv, char const *name) {
  assert(name != NULL);

  char const *home = getenv("HOME");
  if (home != NULL)
    return joinpath4(home, "Library", "Caches", name);

  return NULL;
}

static char *runtimedir(Getenvfn getenv, char const *name) {
  assert(name != NULL);

  char const *tmpdir = getenv("TMPDIR");
  if (tmpdir != NULL)
    return joinpath2(tmpdir, name);

  return joinpath2("/tmp", name);
}

char *getconfigdir(Getenvfn getenv, char const *name) {
  return appsupport(getenv, name);
}

char *getdatadir(Getenvfn getenv, char const *name) {
  return appsupport(getenv, name);
}

char *getcachedir(Getenvfn getenv, char const *name) {
  return caches(getenv, name);
}

char *getruntimedir(Getenvfn getenv, char const *name) {
  return runtimedir(getenv, name);
}
