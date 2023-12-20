#pragma once

#include <filesystem>
#include <optional>

namespace malachi::config {

struct Config {
  std::filesystem::path config_dir;
  std::filesystem::path data_dir;
};

class ConfigBuilder {
public:
  auto build() -> Config;

private:
  std::optional<std::filesystem::path> maybe_config_dir_;
  std::optional<std::filesystem::path> maybe_data_dir_;
};

} // namespace malachi::config
