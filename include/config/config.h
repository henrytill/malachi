#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>

#include "platform.h"

using platform::GetEnvFn;

namespace malachi::config {

struct Config {
    std::filesystem::path config_dir;
    std::filesystem::path data_dir;
};

class ConfigBuilder {
public:
    enum class Result : int8_t {
        Success = 0,
        MissingConfigDir = -1,
        MissingDataDir = -2,
    };

    ConfigBuilder(std::string_view name) : name_{name} {};
    ~ConfigBuilder() = default;

    ConfigBuilder(const ConfigBuilder &) = delete;
    ConfigBuilder(ConfigBuilder &&) = delete;

    ConfigBuilder &operator=(const ConfigBuilder &) = delete;
    ConfigBuilder &operator=(ConfigBuilder &&) = delete;

    ConfigBuilder &with_defaults(GetEnvFn getenv);

    Result build(Config &cfg);

private:
    std::string_view name_;
    std::optional<std::filesystem::path> maybe_config_dir_;
    std::optional<std::filesystem::path> maybe_data_dir_;
};

std::string_view to_string_view(ConfigBuilder::Result result);

} // namespace malachi::config
