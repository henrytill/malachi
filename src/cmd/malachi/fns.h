#pragma once

#include <sys/stat.h>

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

int statuswrite(char const *repopath, char const *sha);
int statusensure(char const *repopath);
