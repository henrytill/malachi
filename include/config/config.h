#pragma once

#include "platform.h"

#define CONFIG_BUILDER_ERROR_VARIANTS             \
  X(MISSING_CONFIG_DIR, -1, "config_dir is NULL") \
  X(MISSING_DATA_DIR, -2, "data_dir is NULL")     \
  X(MIN, -3, NULL)

enum {
#define X(tag, value, description) CONFIG_BUILDER_ERROR_##tag = (value),
  CONFIG_BUILDER_ERROR_VARIANTS
#undef X
};

struct config {
  const char *config_dir;
  const char *data_dir;
};

void config_finish(struct config *config);

struct config_builder;

struct config_builder *config_builder_create(void);

typedef char *getenv_fn(const char *name);

void config_builder_with_defaults(struct config_builder *builder, enum platform p, getenv_fn getenv);

int config_builder_build(struct config_builder *builder, struct config *out);

const char *config_builder_error_to_string(int rc);
