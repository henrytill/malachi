#pragma once

#include <limits.h>
#include <stddef.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#define STATIC_ASSERT(expr) _Static_assert((expr), #expr)

#define NELEM(x) (sizeof(x) / sizeof((x)[0]))

enum {
  MAXHASHLEN = 65,
  MAXOPSIZE = 16,
  MAXFIELDS = 5,
  MAXRECORDSIZE = MAXOPSIZE + (2 * PATH_MAX) + (2 * MAXHASHLEN) + MAXFIELDS,
  MAXQUERYIDLEN = 64,
  MAXQUERYTERMSLEN = 4096,
};

enum {
  Emissingdir = 2,
  Enospace = 3,
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

typedef enum Opcode {
  Opunknown = 0,
  /* Old ASCII protocol operations */
  Opadded,
  Opchanged,
  Opremoved,
  /* New JSON protocol operations */
  Opadd,
  Opremove,
  Opquery,
  /* Common */
  Opshutdown,
} Opcode;

struct Command {
  Opcode op;
  union {
    /* Old ASCII protocol */
    struct {
      char root[PATH_MAX];
      char roothash[MAXHASHLEN];
      char leaf[PATH_MAX];
      char leafhash[MAXHASHLEN];
    } fileop;
    /* New JSON protocol */
    struct {
      char path[PATH_MAX];
    } pathop;
    struct {
      char queryid[MAXQUERYIDLEN];
      char terms[MAXQUERYTERMSLEN];
      char repofilter[PATH_MAX];
    } queryop;
    /* shutdown needs no fields */
  };
};

#define FILEFIELDS   \
  X(fileop.root)     \
  X(fileop.roothash) \
  X(fileop.leaf)     \
  X(fileop.leafhash)

#define PATHOPFIELDS \
  X(pathop.path, "path", 1)

#define QUERYFIELDS                \
  X(queryop.queryid, "queryId", 1) \
  X(queryop.terms, "terms", 1)     \
  X(queryop.repofilter, "repoFilter", 0)

struct Fieldspec {
  size_t const offset;
  size_t const size;
  char const *const name;
  int const required;
  char const *const jsonkey;
};

#define X(field) STATIC_ASSERT(sizeof(((Command *)0)->field) <= INT_MAX); /* NOLINT(bugprone-sizeof-expression) */
FILEFIELDS
#undef X

#define X(field, jsonkey, required) STATIC_ASSERT(sizeof(((Command *)0)->field) <= INT_MAX); /* NOLINT(bugprone-sizeof-expression) */
PATHOPFIELDS
QUERYFIELDS
#undef X

static struct Fieldspec const filefields[] = {
#define X(field) {offsetof(Command, field), sizeof(((Command *)0)->field), #field, 1, NULL},
  FILEFIELDS
#undef X
};

static struct Fieldspec const pathopfields[] = {
#define X(field, jsonkey, required) {offsetof(Command, field), sizeof(((Command *)0)->field), #field, required, jsonkey},
  PATHOPFIELDS
#undef X
};

static struct Fieldspec const queryopfields[] = {
#define X(field, jsonkey, required) {offsetof(Command, field), sizeof(((Command *)0)->field), #field, required, jsonkey},
  QUERYFIELDS
#undef X
};

static struct {
  Opcode const op;
  size_t const namelen;
  char const *const name;
  size_t const nfields;
  struct Fieldspec const *const fields;
} const ops[] = {
#define OP(opcode, name, nfields, fieldspecs) {opcode, sizeof(name) - 1, name, nfields, fieldspecs}
  OP(Opadded, "added", 4, filefields),
  OP(Opchanged, "changed", 4, filefields),
  OP(Opremoved, "removed", 4, filefields),
  OP(Opshutdown, "shutdown", 0, NULL),
#undef OP
};

static struct {
  Opcode const op;
  size_t const namelen;
  char const *const name;
  size_t const nfields;
  struct Fieldspec const *const fields;
} const jsonops[] = {
#define OP(opcode, name, nfields, fieldspecs) {opcode, sizeof(name) - 1, name, nfields, fieldspecs}
  OP(Opadd, "add", 1, pathopfields),
  OP(Opremove, "remove", 1, pathopfields),
  OP(Opquery, "query", 3, queryopfields),
  OP(Opshutdown, "shutdown", 0, NULL),
#undef OP
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
int parsecommand(Parser *p, Command *cmd, int *generation);

/* globals */

extern char const *const appname;
extern int debug;
