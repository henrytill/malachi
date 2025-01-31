#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <variant>

#include "platform.h"

namespace malachi::config {

using platform::GetEnvFn;

enum class ErrorCode : int8_t {
  kMissingDir = 1,
};

struct Error {
  ErrorCode code;
  std::string message;
};

struct Config {
  std::filesystem::path config_dir;
  std::filesystem::path data_dir;
};

using Result = std::variant<Config, Error>;

class Builder {
public:
  Builder(GetEnvFn getenv);
  auto with_defaults() && -> Builder &&;
  auto build() && -> Result;

private:
  GetEnvFn getenv_;
  std::optional<std::filesystem::path> maybe_config_dir_;
  std::optional<std::filesystem::path> maybe_data_dir_;
};

} // namespace malachi::config
