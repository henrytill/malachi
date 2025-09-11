#pragma once

int eprintf(char *fmt, ...);

char *joinpath2(char const *a, char const *b);
char *joinpath3(char const *a, char const *b, char const *c);
char *joinpath4(char const *a, char const *b, char const *c, char const *d);

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
