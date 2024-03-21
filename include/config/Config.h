#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>

#include "Platform.h"

namespace malachi::config {

struct Config {
    std::filesystem::path config_dir;
    std::filesystem::path data_dir;
};

using GetEnv = char *(const char *) noexcept;

class ConfigBuilder {
public:
    enum class Result : int8_t {
        Success = 0,
        MissingConfigDir = -1,
        MissingDataDir = -2,
    };

    ConfigBuilder() = default;
    ~ConfigBuilder() = default;

    ConfigBuilder(const ConfigBuilder &) = delete;
    ConfigBuilder(ConfigBuilder &&) = delete;

    ConfigBuilder &operator=(const ConfigBuilder &) = delete;
    ConfigBuilder &operator=(ConfigBuilder &&) = delete;

    ConfigBuilder &with_defaults(Platform platform, GetEnv getenv);

    Result build(Config &cfg);

private:
    std::optional<std::filesystem::path> maybe_config_dir_;
    std::optional<std::filesystem::path> maybe_data_dir_;
};

std::string to_string(ConfigBuilder::Result result);

} // namespace malachi::config
