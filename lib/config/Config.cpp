#include "Config.h"

namespace malachi::config {

auto ConfigBuilder::build() -> Config {
  auto config_dir = maybe_config_dir_.value_or(std::filesystem::path{});
  auto data_dir = maybe_data_dir_.value_or(std::filesystem::path{});
  return Config{config_dir, data_dir};
}

} // namespace malachi::config
