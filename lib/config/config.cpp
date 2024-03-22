#include "config.h"

#include <cassert>
#include <memory>

#include "platform.h"

using std::filesystem::path;

namespace malachi::config {

ConfigBuilder &ConfigBuilder::with_defaults(GetEnv getenv)
{
    constexpr auto platform = platform::get_platform();
    const auto name = path{"malachi"};
    maybe_config_dir_ = platform::get_config_dir<platform>(getenv, name);
    maybe_data_dir_ = platform::get_data_dir<platform>(getenv, name);
    return *this;
}

ConfigBuilder::Result ConfigBuilder::build(Config &cfg)
{
    if (!maybe_config_dir_.has_value()) {
        return Result::MissingConfigDir;
    }
    if (!maybe_data_dir_.has_value()) {
        return Result::MissingDataDir;
    }
    cfg.config_dir = maybe_config_dir_.value();
    cfg.data_dir = maybe_data_dir_.value();
    return Result::Success;
}

std::string to_string(const ConfigBuilder::Result result)
{
    switch (result) {
    case ConfigBuilder::Result::Success:
        return "Success";
    case ConfigBuilder::Result::MissingConfigDir:
        return "Missing config directory";
    case ConfigBuilder::Result::MissingDataDir:
        return "Missing data directory";
    }
    assert(false);
}

} // namespace malachi::config
