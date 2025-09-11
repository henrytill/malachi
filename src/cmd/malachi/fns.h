#pragma once

#define eprintf(...) (void)fprintf(stderr, __VA_ARGS__)

char *joinpath2(char const *a, char const *b);

char *joinpath3(char const *a, char const *b, char const *c);

char *joinpath4(char const *a, char const *b, char const *c, char const *d);

char const *platform_to_string(void);

char *platform_get_config_dir(platform_getenv_fn getenv, char const *name);

char *platform_get_data_dir(platform_getenv_fn getenv, char const *name);

int config_init(platform_getenv_fn getenv, struct config *config, struct error *err);

void config_finish(struct config *config);

void test_register(struct test_ops const *ops);

int test_run_all(void);

int test_run_by_name(char const *name);
