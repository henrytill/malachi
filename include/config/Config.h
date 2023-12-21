#pragma once

#include <filesystem>
#include <optional>

#include "Platform.h"

namespace malachi::config {

struct Config {
  std::filesystem::path config_dir;
  std::filesystem::path data_dir;
};

using GetEnv = char *(*)(const char *) noexcept;

class ConfigBuilder {
public:
  enum class Result {
    Success = 0,
    MissingConfigDir = -1,
    MissingDataDir = -2,
  };

  ConfigBuilder() = default;
  ~ConfigBuilder() = default;

  ConfigBuilder(const ConfigBuilder &) = delete;
  ConfigBuilder(ConfigBuilder &&) = delete;

  auto operator=(const ConfigBuilder &) -> ConfigBuilder & = delete;
  auto operator=(ConfigBuilder &&) -> ConfigBuilder & = delete;

  auto with_defaults(Platform platform, GetEnv getenv) -> ConfigBuilder &;

  auto build(Config &cfg) -> Result;

private:
  std::optional<std::filesystem::path> maybe_config_dir_;
  std::optional<std::filesystem::path> maybe_data_dir_;
};

auto to_string(ConfigBuilder::Result result) -> const char *;

} // namespace malachi::config
