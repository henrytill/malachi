#include "config.h"

#include <stdlib.h>

struct config_builder {
  const char *maybe_config_dir;
  const char *maybe_data_dir;
};

struct config_builder *config_builder_create(void) {
  struct config_builder *builder = calloc(1, sizeof(*builder));
  builder->maybe_config_dir = NULL;
  builder->maybe_data_dir = NULL;
  return builder;
};

int config_builder_build(struct config_builder *builder, struct config *out) {
  int ret = -1;
  if (builder->maybe_config_dir == NULL) {
    goto out_return;
  }
  if (builder->maybe_data_dir == NULL) {
    goto out_return;
  }
  out->config_dir = builder->maybe_config_dir;
  out->data_dir = builder->maybe_data_dir;
  ret = 0;
  free(builder);
out_return:
  return ret;
};
