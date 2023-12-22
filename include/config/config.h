#pragma once

#include "platform.h"

struct config {
  const char *config_dir;
  const char *data_dir;
};

void config_finish(struct config *config);

struct config_builder;

struct config_builder *config_builder_create(void);

struct config_builder_result {
  enum {
    CONFIG_BUILDER_RESULT_OK = 0,
    CONFIG_BUILDER_RESULT_ERR = -1,
  } tag;
  union {
    struct config ok;
    const char *err;
  } data;
};

struct config_builder_result config_builder_build(struct config_builder *builder);

typedef char *getenv_fn(const char *name);

void config_builder_with_defaults(struct config_builder *builder, enum platform p, getenv_fn getenv);
