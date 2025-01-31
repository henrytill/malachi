#include "config.h"
#include "platform.h"

namespace malachi::config {

using platform::GetEnvFn;

const std::filesystem::path kName = "malachi";
constexpr std::string_view kMissingConfigDirMsg = "maybe_config_dir is empty";
constexpr std::string_view kMissingDataDirMsg = "maybe_data_dir is empty";

Builder::Builder(GetEnvFn getenv) : getenv_{std::move(getenv)} {}

auto Builder::with_defaults() && -> Builder && {
  maybe_config_dir_ = platform::get_config_dir(getenv_, kName);
  maybe_data_dir_ = platform::get_data_dir(getenv_, kName);
  return std::move(*this);
}

auto Builder::build() && -> Result {
  if (not maybe_config_dir_.has_value()) {
    return Error{
      .code = ErrorCode::kMissingDir,
      .message = std::string{kMissingConfigDirMsg},
    };
  }

  if (not maybe_data_dir_.has_value()) {
    return Error{
      .code = ErrorCode::kMissingDir,
      .message = std::string{kMissingDataDirMsg},
    };
  }

  auto config_dir = std::move(maybe_config_dir_).value();
  auto data_dir = std::move(maybe_data_dir_).value();

  return Config{
    .config_dir = std::move(config_dir),
    .data_dir = std::move(data_dir),
  };
}

} // namespace malachi::config
