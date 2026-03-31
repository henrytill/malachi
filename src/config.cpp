#include "config.h"

#include <format>
#include <string>
#include <string_view>
#include <utility>

#include "platform.h"

namespace malachi::config
{

static constexpr auto kName = std::string_view { "malachi" };
static constexpr auto kMissingConfigDirMsg = std::string_view { "Configuration directory could not be determined" };
static constexpr auto kMissingDataDirMsg = std::string_view { "Data directory could not be determined" };
static constexpr auto kMissingCacheDirMsg = std::string_view { "Cache directory could not be determined" };
static constexpr auto kMissingRuntimeDirMsg = std::string_view { "Runtime directory could not be determined" };

auto Config::platform() -> platform::Platform
{
    return platform::get_platform();
}

auto Config::to_string() const -> std::string
{
    return std::format(
        "platform: {}\nconfig_dir: {}\ndata_dir: {}\ncache_dir: {}\nruntime_dir: {}\n",
        platform::to_string_view(platform()),
        config_dir.string(),
        data_dir.string(),
        cache_dir.string(),
        runtime_dir.string());
}

Builder::Builder(platform::GetEnvFn getenv)
    : getenv_ { std::move(getenv) }
{
}

auto Builder::with_defaults() && -> Builder &&
{
    maybe_config_dir_ = platform::get_config_dir(getenv_, kName);
    maybe_data_dir_ = platform::get_data_dir(getenv_, kName);
    maybe_cache_dir_ = platform::get_cache_dir(getenv_, kName);
    maybe_runtime_dir_ = platform::get_runtime_dir(getenv_, kName);
    return std::move(*this);
}

auto Builder::build() && -> Result
{
    if (not maybe_config_dir_.has_value())
    {
        return Error {
            .code = ErrorCode::kMissingDir,
            .message = std::string { kMissingConfigDirMsg },
        };
    }

    if (not maybe_data_dir_.has_value())
    {
        return Error {
            .code = ErrorCode::kMissingDir,
            .message = std::string { kMissingDataDirMsg },
        };
    }

    if (not maybe_cache_dir_.has_value())
    {
        return Error {
            .code = ErrorCode::kMissingDir,
            .message = std::string { kMissingCacheDirMsg },
        };
    }

    if (not maybe_runtime_dir_.has_value())
    {
        return Error {
            .code = ErrorCode::kMissingDir,
            .message = std::string { kMissingRuntimeDirMsg },
        };
    }

    return Config {
        .config_dir = std::move(maybe_config_dir_.value()),
        .data_dir = std::move(maybe_data_dir_.value()),
        .cache_dir = std::move(maybe_cache_dir_.value()),
        .runtime_dir = std::move(maybe_runtime_dir_.value()),
    };
}

} // namespace malachi::config
