#pragma once

#include <limits.h>
#include <stddef.h>
#include <sys/stat.h>
#include <sys/types.h>

#define STATIC_ASSERT(expr) _Static_assert((expr), #expr)

#define NELEM(x) (sizeof(x) / sizeof((x)[0]))

enum {
	MAXHASHLEN = 65,
	MAXOPSIZE = 16,
	MAXFIELDS = 5,
	MAXRECORDSIZE = MAXOPSIZE + (2 * PATH_MAX) + (2 * MAXHASHLEN) + MAXFIELDS,
};

enum {
	Emissingdir = 2,
	Enospace = 3,
};

enum {
	Opunknown = 0,
	Opadded,
	Opchanged,
	Opremoved,
	Opshutdown,
};

typedef struct Error Error;
typedef struct Config Config;
typedef struct Filter Filter;
typedef struct Test Test;
typedef struct Database Database;
typedef struct Parser Parser;
typedef struct Command Command;

typedef char *Getenvfn(char const *name);

struct Error {
	int rc;
	char const *msg;
};

struct Config {
	char *configdir;
	char *datadir;
	char *cachedir;
	char *runtimedir;
};

struct Filter {
	char const *name;
	char const **exts;
	int (*extract)(char const *input, char **output);
	char const *(*version)(void);
};

struct Test {
	char const *name;
	int (*run)(void);
};

struct Command {
	int op;
	char root[PATH_MAX];
	char roothash[MAXHASHLEN];
	char leaf[PATH_MAX];
	char leafhash[MAXHASHLEN];
};

int eprintf(char *fmt, ...);
void loginfo(char const *fmt, ...);
void logerror(char const *fmt, ...);
void logdebug(char const *fmt, ...);

char *joinpath2(char const *a, char const *b);
char *joinpath3(char const *a, char const *b, char const *c);
char *joinpath4(char const *a, char const *b, char const *c, char const *d);
int mkdirp(char const *path, mode_t mode);

char *platformstr(void);
char *getconfigdir(Getenvfn getenv, char const *name);
char *getdatadir(Getenvfn getenv, char const *name);
char *getcachedir(Getenvfn getenv, char const *name);
char *getruntimedir(Getenvfn getenv, char const *name);

int configinit(Getenvfn getenv, Config *config, Error *err);
void configfree(Config *config);

void filteradd(Filter const *ops);
Filter const *filterget(char const *ext);
Filter const **filterall(void);

void testadd(Test const *ops);
int testall(void);
int testone(char const *name);

Database *dbcreate(Config const *config, Error *err);
void dbdestroy(Database *db);
int dbensure(Database *db, Error *err);
char *dbrepoget(Database *db, char const *repopath);
int dbreposet(Database *db, char const *repopath, char const *sha);

int statuswrite(char const *runtimedir, char const *repopath, char const *sha);
int statusensure(char const *runtimedir, char const *repopath);

Parser *parsercreate(size_t bufsize);
void parserdestroy(Parser *p);
void parserreset(Parser *p);
ssize_t parserinput(Parser *p, int fd);
int parsecommand(Parser *p, Command *cmd);

/* globals */

extern char const *const appname;
extern int debug;
