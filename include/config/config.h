#pragma once

#include "platform.h"

struct config {
  const char *config_dir;
  const char *data_dir;
};

void config_finish(struct config *config);

struct config_builder;

struct config_builder *config_builder_create(void);

int config_builder_build(struct config_builder *builder, struct config *out);

typedef char *getenv_fn(const char *name);

void config_builder_with_defaults(struct config_builder *builder, enum platform p, getenv_fn getenv);
