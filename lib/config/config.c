#include "config.h"
#include "platform.h"

#include <stdio.h>
#include <stdlib.h>

static const char *const MALACHI_DIR = "malachi";

void config_finish(struct config *config) {
  free(config->config_dir);
  free(config->data_dir);
}

struct config_builder {
  char *maybe_config_dir;
  char *maybe_data_dir;
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
    goto out_free_builder;
  }
  if (builder->maybe_data_dir == NULL) {
    goto out_free_builder;
  }
  out->config_dir = builder->maybe_config_dir;
  out->data_dir = builder->maybe_data_dir;
  ret = 0;
out_free_builder:
  free(builder);
  return ret;
};

static char *joinpath2(enum platform p, const char *a, const char *b) {
  const char sep = (p == PLATFORM_WINDOWS) ? '\\' : '/';
  size_t len = (size_t)snprintf(NULL, 0, "%s%c%s", a, sep, b);
  char *ret = calloc(++len, sizeof(char)); // incr for terminator
  if (ret == NULL) {
    return NULL;
  }
  (void)snprintf(ret, len, "%s%c%s", a, sep, b);
  return ret;
}

static char *joinpath3(enum platform p, const char *a, const char *b, const char *c) {
  const char sep = (p == PLATFORM_WINDOWS) ? '\\' : '/';
  size_t len = (size_t)snprintf(NULL, 0, "%s%c%s%c%s", a, sep, b, sep, c);
  char *ret = calloc(++len, sizeof(char)); // incr for terminator
  if (ret == NULL) {
    return NULL;
  }
  (void)snprintf(ret, len, "%s%c%s%c%s", a, sep, b, sep, c);
  return ret;
}

static char *joinpath4(enum platform p, const char *a, const char *b, const char *c, const char *d) {
  const char sep = (p == PLATFORM_WINDOWS) ? '\\' : '/';
  size_t len = (size_t)snprintf(NULL, 0, "%s%c%s%c%s%c%s", a, sep, b, sep, c, sep, d);
  char *ret = calloc(++len, sizeof(char)); // incr for terminator
  if (ret == NULL) {
    return NULL;
  }
  (void)snprintf(ret, len, "%s%c%s%c%s%c%s", a, sep, b, sep, c, sep, d);
  return ret;
}

static char *get_windows_config_dir(getenv_fn getenv) {
  char *app_data = getenv("APPDATA");
  if (app_data == NULL) { return NULL; }
  char *ret = joinpath2(PLATFORM_WINDOWS, app_data, MALACHI_DIR);
  return ret;
}

static char *get_windows_data_dir(getenv_fn getenv) {
  char *local_app_data = getenv("LOCALAPPDATA");
  if (local_app_data == NULL) { return NULL; }
  char *ret = joinpath2(PLATFORM_WINDOWS, local_app_data, MALACHI_DIR);
  return ret;
}

static char *get_macos_support_dir(getenv_fn getenv) {
  char *home = getenv("HOME");
  if (home == NULL) { return NULL; }
  char *ret = joinpath4(PLATFORM_MACOS, home, "Library", "Application Support", MALACHI_DIR);
  return ret;
}

static char *get_xdg_config_home(getenv_fn getenv) {
  char *xdg_config_home = getenv("XDG_CONFIG_HOME");
  if (xdg_config_home != NULL) {
    char *ret = joinpath2(PLATFORM_LINUX, xdg_config_home, MALACHI_DIR);
    return ret;
  }
  char *home = getenv("HOME");
  char *ret = joinpath3(PLATFORM_LINUX, home, ".config", MALACHI_DIR);
  return ret;
}

static char *get_xdg_data_home(getenv_fn getenv) {
  char *xdg_data_home = getenv("XDG_DATA_HOME");
  if (xdg_data_home != NULL) {
    char *ret = joinpath2(PLATFORM_LINUX, xdg_data_home, MALACHI_DIR);
    return ret;
  }
  char *home = getenv("HOME");
  char *ret = joinpath4(PLATFORM_LINUX, home, ".local", "share", MALACHI_DIR);
  return ret;
}

void config_builder_with_defaults(struct config_builder *builder, enum platform p, getenv_fn getenv) {
  switch (p) {
  case PLATFORM_WINDOWS: {
    builder->maybe_config_dir = get_windows_config_dir(getenv);
    builder->maybe_data_dir = get_windows_data_dir(getenv);
  } break;
  case PLATFORM_MACOS: {
    char *support_dir = get_macos_support_dir(getenv);
    builder->maybe_config_dir = support_dir;
    builder->maybe_data_dir = support_dir;
  } break;
  case PLATFORM_LINUX:
  case PLATFORM_UNKNOWN:
  default: {
    builder->maybe_config_dir = get_xdg_config_home(getenv);
    builder->maybe_data_dir = get_xdg_data_home(getenv);
  } break;
  }
}
