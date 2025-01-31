#include "config.h"

#include <format>

#include "platform.h"

namespace malachi::config {

using platform::GetEnvFn;

static const std::filesystem::path kName = "malachi";
static constexpr std::string_view kMissingConfigDirMsg = "maybe_config_dir is empty";
static constexpr std::string_view kMissingDataDirMsg = "maybe_data_dir is empty";

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

  return Config{
    .config_dir = std::move(maybe_config_dir_.value()),
    .data_dir = std::move(maybe_data_dir_.value()),
  };
}

auto Config::platform() -> platform::Platform {
  return platform::get_platform();
}

auto Config::to_string() const -> std::string {
  return std::format("platform: {}\nconfig_dir: {}\ndata_dir: {}\n",
                     platform::to_string_view(platform()),
                     config_dir.string(),
                     data_dir.string());
}

} // namespace malachi::config
