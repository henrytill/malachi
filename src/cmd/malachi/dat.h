#pragma once

/* IWYU pragma: always_keep */

typedef struct Error Error;

struct Error {
	int rc;
	char const *msg;
};

typedef char *Getenvfn(char const *name);

enum {
	EMISSINGDIR = 1,
};

typedef struct Config Config;

struct Config {
	char *configdir;
	char *datadir;
	char *cachedir;
};

typedef struct Filter Filter;

struct Filter {
	char const *name;
	char const **exts;
	int (*extract)(char const *input, char **output);
	char const *(*version)(void);
};

typedef struct Test Test;

struct Test {
	char const *name;
	int (*run)(void);
};

extern int debug;
extern char const *const appname;
